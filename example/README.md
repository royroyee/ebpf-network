# Practice


### 사전 준비
```shell
sudo apt-get update
sudo apt install clang
sudo apt-get install linux-tools-common
sudo apt-get install linux-tools-5.15.0-71-generic # linux versinon 체크 필요

sudo apt-get install libbpf-dev
sudo apt-get install linux-headers-$(uname -r)
sudo apt-get install gcc-multilib

sudo apt install python3-bpfcc 

```
### [golang 설치](https://tecadmin.net/how-to-install-go-on-ubuntu-20-04/) 
```
sudo apt install golang # 최신버전이 아닐 수 있음 (1.18깔렸었음)
go version
```

#### Make 설치
```shell
sudo apt install make
```

---

### 헤더 파일 참고 시 유용한 사이트
- [linux kernel](https://elixir.bootlin.com/linux/v5.15.71/source/tools/lib/bpf) 

## ebpf 코드 실행 방법

### 1. vmlinux.h 이용 시
```shell
bpftool btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h
```
- 이유,개념 및 흐름은 이후 다룰 예정

### 2. clang 컴파일
```shell
clang -O2 -target bpf -c network.bpf.c -o network.bpf.o
```

### `bpf_trace_printk()` 를 통해 확인하고 싶을때
```shell
sudo cat /sys/kernel/debug/tracing/trace_pipe
```


### eBPF Map
(작성 예정)


---


### TC code


#### 해당 인터페스의 qdisc 확인하기
```
tc qdisc show dev ens160
```

#### 인터페이스의 index 확인하기
```cgo
$ cat /sys/class/net/br0/ifindex
13
```

#### 1. Create a clsact qdisc on network interface
```
sudo tc qdisc add dev ens160 clsact
```


#### 2. Create an ingress filter 
```
sudo tc filter add dev ens160 ingress bpf da obj drop-icmp.o sec ingress
```
- bpf : eBPF 프로그램을 사용하여 필터 구현
- da : 디스크립터로부터 eBPF 오브젝트 로드
- obj drop-icmp.o : 해당 eBPF 오브젝트 파일을 로드
- sec ingress : ingress 섹션을 사용하여 필터 적용

#### 3. Check ingress filter
```
sudo tc filter show dev ens160 ingress
```


#### 추가 내용(필터 우선순위 등)
- [작성 예정](https://gist.github.com/anfredette/732eeb0fe519c8928d6d9c190728f7b5)


### 삭제하기
```cgo
sudo tc filter del dev ens160 ingress pref <pref number>
```
- tc filter show 로 나온 preference number

#### 모든 것을 삭제하기
```cgo
sudo tc qdisc del dev ens160 clsact
```
- 기존에 clasct qdisc 가 있다면 유의할 것

### Example
- [ex1 : ICMP Drop](https://github.com/royroyee/ebpf-network/tree/main/example/ex1)
- [ex2 : Redirection](https://github.com/royroyee/ebpf-network/tree/main/example/ex2)
- [ex3 : Create ARP Reply Packet]()
 

---

### XDP code
(작성 예정)




