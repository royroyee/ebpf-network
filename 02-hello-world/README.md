# Hello World 


## BCC's Python Library 를 이용한 Hello World (Basic)
```
#!/usr/bin/python

from bcc import BPF

program = r"""
int hello(void *ctx) {
    bpf_trace_printk("Hello World!");
    return 0;
}
"""
b = BPF(text=program)
syscall = b.get_syscall_fnname("execve")
b.attach_kprobe(event=syscall, fn_name="hello")
b.trace_print()

```
- 'program' 변수에 C 코드 형식으로 eBPF 프로그램을 정의한다.
  - hello() 를 정의
  - `bfp_trace_printk()` 를 호출하여 Hello World 라는 메시지 작성한다.
    - 가상 파일 위치인 /sys/kernel/debug/tracing/trace_pipe 로 출력을 보낸다.
    - python 코드는 이 파일 위치에서 출력을 읽는다.


- `BPF(text=program)` : BPF 객체를 생성한다.
  - 이 객체는 eBPF 프로그램을 컴파일하고 커널에 로드하는 역할을 담당한다.

- `attach_kprobe(event=syscall, fn_name="hello"` : eBPF 프로그램을 execve() 시스템 호출에 연결한다.
  - `fn_name="hello"`: hello() 함수를 execve() 시스템 콜에 연결하겠다는 의미
  - execve() 시스템 호출이 발생할 때마다 hello 함수가 실행된다.

- `trace_print()` : 커널에서 생성된 추적 정보를 읽고  출력한다.

### 동작 방식
![hello world.png](..%2F..%2F..%2FeBPF%2Fhello%20world.png)

1. Python 프로그램은 C 코드를 컴파일하고 커널에 로드, kprove에 execve() 시스템 콜을 연결한다.
2. 해당 머신에서 exeve()를 호출하는 애플리케이션은 eBPF 프로그램의 hello() 함수를 트리거하게된다.
3. Python 프로그램은 trace 메시지를 읽고 유저에게 출력해준다.

### 알 수 있는 eBPF 의 장점
- eBPF 프로그램이 로드되고 이벤트에 연결되면, 기존 프로세스에서 생성된 이벤트에 의해 트리거 될 수 있다.
 - 이는 eBPF 프로그램이 시스템의 동작을 동적으로 변경할 수 있으며 기존 프로세스를 다시 시작하거나 재부팅할 필요가 없다는 개념을 강화해준다.
- 다른 응용프로그램은 수정할 필요없이 eBPF 에서 처리할 수 있다.

---

## BPF MAPS
- Map 은 eBPF 프로그램과 User space 에서 접근할 수 있는 데이터 구조이다.
- 여러 eBPF 프로그램 간에 데이터를 공유하거나 사용자 공간 애플리케이션과 커널에서 실행되는 eBPF 코드 간에 통신하는 데 사용될 수 있다.

### 사용 사례
- Monitoring and Observavility : 맵은 커널 내에서 실행되는 eBPF 프로그램이 수집한 데이터를 사용자 공간 애플리케이션에 전달하는 데 사용할 수 있다.
  - 이를 통해 모니터링 및 관측 도구에서 실시간으로 시스템 상태를 분석하고 통계를 수집할 수 있다.
- Sequrity and Network Filtering : 맵은 네트워크 패킷을 필터링하거나 보안 정책을 적용하는 데 사용될 수 있다.
  - 예를 들어 특정 IP 주소 또는 포트에 대한 액세스 제어, 패킷 검사 및 차단, 네트워크 연결 추적 등에 활용될 수 있다.
- Performance Optimizatinon : 맵은 성능 최적화를 위해 사용될 수 있다.
  - 맵을 이용한 캐시 메커니즘 구현, 데이터 공유하여 계산 비용 줄이기 등


### Example - Hash Table Map
- 이 예제도 위 예제와 동일하게 execve() 시스템 콜 진입점에 대한 kprobe 에 연결된다.
- 해시 테이블을 key-value 쌍으로 채운다.
  - key : 사용자 ID
  - value : 해당 사용자 ID로 실행된 프로세스에서 execve() 가 호출된 횟수
  - 즉 각 사용자 별로 프로그램이 실행된 횟수를 보여준다.

####  Code
```shell

#!/usr/bin/python3  
from bcc import BPF
from time import sleep

program = r"""
BPF_HASH(counter_table); # c언어 표준에선 사용할 수 없는 문법이나, BCC의 고유 문법으로 지원

int hello(void *ctx) {
  u64 uid;
  u64 counter = 0;
  u64 *p;
  
  uid = bpf_get_current_uid_gid() & 0xFFFFFFFF; # 상위 32비트를 모두 0으로 만드는 마스크 연산
  p = counter_table.lookup(&uid);
  if (p != 0) {
    counter = *p;
  }
  counter++;
  counter_table.update(&uid, &counter);
  return 0;
}
"""


b = BPF(text=program)
syscall = b.get_syscall_fnname("execve")
b.attach_kprobe(event=syscall, fn_name="hello")

# Attach to a tracepoint that gets hit for all syscalls 
# b.attach_raw_tracepoint(tp="sys_enter", fn_name="hello")

while True:
    sleep(2)
    s = ""
    for k,v in b["counter_table"].items():
        s += f"ID {k.value}: {v.value}\t"
    print(s)


```

- `BPF_HASH()` : BCC 매크로이다. 해시 테이블 맵인 `counter_table` 을 정의한다.
- `bpf_get_current_uid_gid()` : 이 함수를 사용하여 현재 실행 중인 프로세스의 유저 ID를 가져온다. 
  - 반환된 64비트 값 중 가장 낮은 32비트가 유저 ID를 나타내기 때문에, 마스크 연산을 통해 얻음.
- `counter_table.lookup(&uid)` : 해시 테이블에서 유저 ID(uid)와 일치하는 키를 가진 항목을 찾는다.
  - 이때 반환 값은 해당 키에 대한 값의 포인터이다.
  - 만약 해시 테이블에 해당 유저 ID 에 항목이 없는 경우 당연히 p 는 0이 될것이고 counter 값은 0으로 유지된다.
- `counter_table.update(&uid, &counter)` : counter 값을 해당 사용자 ID 테이블 값에 업데이트 한다.


- 파이썬 코드는 위 예제와 동일하나 while 루프가 추가되었다.(해시 테이블)
  - 무한 루프로 돌면서 2초마다(sleep(2))) 출력을 확인한다.
  - BCC는 해시 테이블을 나타내는 python 객체를 자동으로 생성한다.(b에 이미 생성됨)

### Example - Ring Buffer Maps
![링 버퍼.png](img%2F%EB%A7%81%20%EB%B2%84%ED%8D%BC.png)

- ring buffer 를 이용하면 더 정교한 작업이 가능해진다.

#### Code

```
#!/usr/bin/python3  
from bcc import BPF

program = r"""
BPF_PERF_OUTPUT(output); 
 
struct data_t {     
   int pid;
   int uid;
   char command[16];
   char message[12];
};
 
int hello(void *ctx) {
   struct data_t data = {}; 
   char message[12] = "Hello World";
 
   data.pid = bpf_get_current_pid_tgid() >> 32;
   data.uid = bpf_get_current_uid_gid() & 0xFFFFFFFF;
   
   bpf_get_current_comm(&data.command, sizeof(data.command));
   bpf_probe_read_kernel(&data.message, sizeof(data.message), message); 
 
   output.perf_submit(ctx, &data, sizeof(data)); 
 
   return 0;
}
"""

b = BPF(text=program) 
syscall = b.get_syscall_fnname("execve")
b.attach_kprobe(event=syscall, fn_name="hello")
 
def print_event(cpu, data, size):  
   data = b["output"].event(data)
   print(f"{data.pid} {data.uid} {data.command.decode()} {data.message.decode()}")
 
b["output"].open_perf_buffer(print_event) 
while True:   
   b.perf_buffer_poll()
  
```
- `BPF_PERF_OUTPUT` : BCC 매크로, user space로 메시지를 전달하기 위해 맵을 생성
  - 여기서는 output 이라는 이름을 사용

- struct data_t : pid,uid, 그리고 running 중인 명령어를 담을 배열 command, 그리고 메시지를 담을 mesaage 배열을 정의

- `bpf_get_current_pid_tgid()` : 현재 실행 중인 eBPF 프로그램을 트리거한 프로세스의 ID를 가져오는 헬퍼 함수
  - 64비트 값을 반환한다. 
    - 상위 32 비트 : pid
    
- `bpf_get_current_uid_gid()` : 이 함수를 사용하여 현재 실행 중인 프로세스의 유저 ID를 가져온다.
  - 반환된 64비트 값 중 가장 낮은 32비트가 유저 ID를 나타내기 때문에, 마스크 연산을 통해 얻음.

- `bpf_get_current_comm()` : execve() 시스템 콜을 한 프로세스에서 실행 중인 실행 파일(또는 command)의 이름을 가져오기 위한 헬퍼 함수
  - 숫자가 아닌 문자열 반환
  - c언어는 "="을 사용하여 문자열을 할당할 수 없으므로, data.command 주소를 전달

- `bpf_probe_read_kernel` : eBPF 프로그램이 커널 메모리에 접근하여 데이터를 읽을 수 있도록 돕는 헬퍼 함수
  - message 라는 주소에서 data.message 에 해당하는 길이만큼 데이터를 커널 메모리로부터 안전하게 일겅와 data.message 에 저장

- `output.perf_sublmit()`:data 구조체가 채워진 상태에서 해당 데이터를 output 맵에 제출한다.
  - 이를 통해 user space 애플리케이션에서 접근할 수 있게 된다.
> 여기서 output map은 커널에서 실행되는 eBPF 프로그램과 user space 애플리케이션 간의 통신 채널로 작동된다.

### Ring Buffer를 사용했을 때의 차이점
![링 버퍼 작동 방식.png](img%2F%EB%A7%81%20%EB%B2%84%ED%8D%BC%20%EC%9E%91%EB%8F%99%20%EB%B0%A9%EC%8B%9D.png)

- 이전 예제와는 달리 중앙의 trace pipe 대신 프로그램이 자체적으로 사용하기 위해 생성한 `output` 이라는 ring buffer map 을 통해 데이터가 전달된다.
  - 기존에는 `/sys/kernel/debug/tracing/trace_pipe` 로 전달되었었다. 실제로 cat 명령어등으로 확인해보면 전달되지 않은 것을 확인할 수 있다.
- 이 맵을 통해 데이터가 전송되기 때문에 여러 개의 이벤트가 동시에 발생이 가능하며, 출력은 순서대로 버퍼에 쌓이게 된다.
  - 때문에 사용자는 필요할 때마다 출력을 읽을 수 있게 된다.



### Example - Tail call