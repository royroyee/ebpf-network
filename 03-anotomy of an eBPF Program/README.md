# Anatomy of an eBPF Program
eBPF 프로그램이 소스코드에서 실행까지 거치는 단계

### 단계
1. C (or rust) 소스 코드는 eBPF 바이트코드로 컴파일 된다.
2. eBPF 바이트코드는 JIT 컴파일이나 기계어 명령어로 해석된다.


### eBPF Virtual Machine


### eBPF Registers

### eBPF Instructions


---

### eBPF "Hello World" for a Network Interface
2장 hello world 는 `kprobe`에 의해 trace "Hello World" 생성 예제였다면 이번에는 네트워크 패킷의 도착에 의해 트리거 되는 프로그램 예제

- 패킷 처리는 eBPF의 매우 일반적인 응용 분야이다.
- 이 예제는 아주 기본적으로 네트워크 인터페이스로 수신되는 모든 데이터 패킷에 대해 트리거 되는 eBPF 프로그램의 기본 아이디어의 이해를 돕는다.
  - 해당 패킷의 내용을 검사하고 수정도 가능하며, 커널이 그 패킷을 처리할 방법에 대한 결정 또는 처리 결과를 내릴 수 있다.
  - 리다이렉션, 포워딩 등

#### Code
```
#include <linux/bpf.h>
#include <bpf/bpf_helpers.h>

int counter = 0;

SEC("xdp")
int hello(struct xdp_md *ctx) {
    bpf_printk("Hello World %d", counter);
    counter++; 
    return XDP_PASS;
}

char LICENSE[] SEC("license") = "Dual BSD/GPL";
```

- counter : 전역 변수
- `SEC()` : `xdp` 라는 섹션을 정의 
  - xdp : eXpress Data Path 의 약자로 linux 커널에서 실행되는 BPF 프로그램의 일종
    - XDP 프로그램은 네트워크 인터페이스(or 가상 인터페이스)에 연결되며, 네트워크 패킷을 처리하고 필터링하는데 사용된다.
- `bpf_printk()` : 문자열을 출력하기 위한 헬퍼 함수
  - `bpf_trace_printk()` 는 BCC 버전, 위는 libbpf 버전이다.
- `XDP_PASS` : 커널에게 이 네트워크 패킷을 정상적으로 처리하라는 판정을 내리는 것을 의미

> 네트워크 패밋이 네트워크 인터페이스로 들어오는 순간 XDP 이벤트가 트리거 되는 예제
