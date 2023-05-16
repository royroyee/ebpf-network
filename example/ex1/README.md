# Example 1

### [Traffic Control 에 대해서](https://github.com/royroyee/ebpf-network/tree/main/networking/traffic-control#traffic-control-tc)


## tc-bpf example - Drop ICMP
- [참고 자료](https://gist.github.com/anfredette/732eeb0fe519c8928d6d9c190728f7b5)

### drop-icmp.c

```cgo
#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <arpa/inet.h>

__attribute__((section("ingress"), used))
int drop(struct __sk_buff *skb) {
    const int l3_off = ETH_HLEN;                      // IP header offset
    const int l4_off = l3_off + sizeof(struct iphdr); // L4 header offset

    void *data = (void*)(long)skb->data;
    void *data_end = (void*)(long)skb->data_end;
    if (data_end < data + l4_off)
        return TC_ACT_OK;

    struct ethhdr *eth = data;
    if (eth->h_proto != htons(ETH_P_IP))
       return TC_ACT_OK;

    struct iphdr *ip = (struct iphdr *)(data + l3_off);
    if (ip->protocol != IPPROTO_ICMP)
        return TC_ACT_OK;

    return TC_ACT_SHOT;
}
```
- `__attribute__((section("ingress"), used))` : 이 eBPF 프로그램이 ingress 섹션에 배치되도록 지정
  - `SEC(ingress)` 와 동일한 역할을 수행 [참고](https://stackoverflow.com/questions/67553794/what-is-variable-attribute-sec-means)


 
- `const int l3_off = ETH_HLEN` : l3_off 를 ETH_HLEN(이더넷 헤더 길이)으로 지정 [ETH_HLEN](https://elixir.bootlin.com/linux/v5.15.71/source/include/uapi/linux/if_ether.h#L34)
- `const int l4_off = l3_off + sizeof(struct iphdr)` : ICMP 패킷 마지막을 가리키기 위해 l3_off + iphdr 
  - ICMP 는 L3 이나, IP 헤더 뒤에 있으므로

```cgo
void *data = (void*)(long)skb->data;
void *data_end = (void*)(long)skb->data_end;
```
- `data`:  `skb`의 데이터 버퍼를 가리키는 포인터
- `data_end` : `skb 데이터 버퍼의 끝을 가리키는 포인터

```cgo
if (data_end < data + l4_off)
   return TC_ACT_OK;
```
- 데이터의 끝이 l4_off 를 더한 것보다 작다는 건 L4 헤더가 존재하지 않는 것을 의미한다.
  - ICMP 패킷을 버려하므로 이땐 `TC_ACT_OK` 를 반환한다.


```cgo
struct ethhdr *eth = data;
  if (eth->h_proto != htons(ETH_P_IP))
    return TC_ACT_OK;
```
- `*eth` : `data` 포인터를 이더넷 헤더 구조체인 `ethhdr 포인터로 캐스팅
- `h_proto`: 이더넷 헤더의 타입필드, IP,IPv6,ARP 등을 구분한다. [ethhdr](https://elixir.bootlin.com/linux/v5.15.71/source/include/uapi/linux/if_ether.h#L171)
- `htons` : 네트워크 프로그래밍에서 IP,TCP,UDP 등과 같은 프로토콜 헤더의 필드를 설정할 때 자주 사용
  - 네트워크 바이트 순서로 변환해준다.
  - `<arpa/inet.h>` 헤더파일에 선언되어있다. 주소 변환 기능을 사용할 경우 사용하는 헤더
- 즉 이 조건문은 IP 타입이 아니면 패킷을 그대로 전송하라는 의미

```cgo
struct iphdr *ip = (struct iphdr *)(data + l3_off);
```
- `data` 포인터에 l3_off 값을 더해 ip 헤더 를 가리키게 하고, 그 이후 `iphdr` 포인터인 ip 에 캐스팅한다.

```cgo
if (ip->protocol != IPPROTO_ICMP)
    return TC_ACT_OK;
```
- `IPPROTO_ICMP` 를 이용하여 ICMP 프로ㅗ콜인지 확인한다.
  - ICMP 프로토콜이 아니라면 마찬가지로 패킷을 그대로 전달하게 한다.

> 이 모든 경우에 해당하지 않을 경우에만 `TC_ACT_SHOT` 을 리턴한다.



### 실행 방법

#### compile
```
$ clang -O2 -target bpf -c drop-icmp.c -o drop-icmp.o
```

#### 1. Create a clsact qdisc on network interface
```
sudo tc qdisc add dev ens160 clsact
```

#### 참고 : 해당 인터페스의 qdisc 확인하기
```
tc qdisc show dev ens160
```

#### 2. Create an ingress filter to drop icmp packets
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

