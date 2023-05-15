# Introduction

참고자료 : [Learning eBPF](https://github.com/lizrice/learning-ebpf) 

## eBPF(Extended Berkeley Packet Filter)란?
- 커널 내에서 실행되는 프로그램
- 네트워크, 보안, 디버깅 등 다양한 용도로 사용
- eBPF 프로그램은 안전하게 실행될 수 있도록 검증되며, 커널의 다양한 부분에서 실행 가능
- 유연하게 확장이 가능하며 새로운 기능을 추가하기 위해 **커널을 수정할 필요 없이 새로운 프로그램을 작성할 수 있다.**

### 특징
- 안정성, 보안성 : eBPF 프로그램은 검증된 코드만이 실행되도록 보장되어 있다. 떄문에 시스템의 안정성 및 보안성을 유지할 수 있따.
- 효율성, 속도 : eBPF는 커널 내에서 실행되기 때문에 매우 빠르고 효율적으로 동작한다. 

### 용도
- 네트워크 패킷 실시간 분석
- 시스템 콜 추적
- 보안 정책 적용 


---
### Hook
- 특정 지점 또는 이벤트에 연결하는 것을 의미
- 특정 이벤트가 발생할 때 해당 이벤트를 감지하고 원하는 동작을 수행하기 위해 사용되는 개념

### eBPF in Cloud Native Environments
컨테이너, 쿠버네티스, Lambda와 같은 클라우드 네이티브 접근 방식에서 각각의 서버는 커널을 실행한다.
- 컨테이너에서 애플리케이션이 실행되는 경우 만약 **동일한 (가상)머신에서 실행되는 경우 동일한 커널을 공유**한다.
  - 이 커널에 eBPF 프로그램을 적용하면 해당 노드의 모든 컨테이너화된 워크로드개 해당 eBPF 프로그램에게 가시적으로 표시된다.
- 네트워크 기능이 사이드카 방식으로 구현될 경우 애플리케이션 컨테이너와 모든 트래픽은 네트워크 스택을 통해 커널을 거쳐 네트워크 프록시 컨테이너에 도달해야 한다.
  - 이로 인해 해당 트래픽에 레이턴시가 추가된다.
  - 이러한 것들을 eBPF 를 통해 네트워크 성능을 향상 시킬 수 있다.
![네트워크-ebpf 전.png](..%2F..%2F..%2FeBPF%2F%EB%84%A4%ED%8A%B8%EC%9B%8C%ED%81%AC-ebpf%20%EC%A0%84.png)
  - eBPF 사용 전, 사이드카 방식을 통한 네트워크 패킷 흐름


### BCC(BPF Compiler Collection)
- eBPF 프로그래밍을 위한 도구 모음
- Python , C/C++ 를 기반으로 한 고수준 라이브러리
- eBPF 프로그램 개발, 디버깅 및 성능 분석을 쉽게 할 수 있도록 도와준다.
- BCC는 BPF 프로그램을 컴파일 및 로드하며, 커널과의 상호작용을 위한 기능을 제공한다.


### kprobe
- 리눅스 커널의 디버깅 및 성능 분석을 위한 기술 중 하나
- 실행 중인 프로세스의 동작을 추척하고 시스템 콜 호출 및 커널 함수 호출과 같은 이벤트 모니터링 가능
  - 이를 통해 개발자는 시스템 동작 분석 및 문제 발생 원인을 파악할 수 있다.
- `execve()` 시스템 콜의 진입점 또는 do_execve() 커널 함수에 연결될 수 있다.
  - 프로세스가 실행되기 전 또는 실행 중에 해당 함수에서 발생하는 이벤트를 모니터링 할 수 있다.
  - `execve()` : 리눅스에서 프로그램을 실행하는 시스텐 콜
    - 이 함수를 호출하면 커널은 새로운 프로그램을 실행하고 현재 프로세스의 메모리와 상태를 새로운 프로그램으로 교체한다.

- eBPF 와 함께 사용하여 더 상세하게 분석할 수 있다