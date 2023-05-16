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

### TC code
(작성 예정)



### XDP code
(작성 예정)




### `bpf_trace_printk()` 를 통해 확인하고 싶을때
```shell
sudo cat /sys/kernel/debug/tracing/trace_pipe
```

