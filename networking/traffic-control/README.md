# Traffic Control (TC)
리눅스 커널의 서브 시스템으로 네트워크 트래픽의 스케쥴링을 조절한다.
또한 패킷 처리와 우선순ㄴ위 설정에 대한 세밀한 제어를 가능하게 하며, 네트워크 트래픽을 특정 요구사항에 맞게 조절할 수 있도록 한다.


### sk-buff 데이터 구조
- TC에서 네트워크 패킷은 sk-buff 데이터 구조로 표현된다.
- 이는 커널의 네트워크 스택 전반에서 사용되는 구조체이다.
- TC 하위시스템에 연결된 eBPF 프로그램은 컨텍스트 파라미터로 sk_buff 구조체에 대한 포인터를 받게 된다.
  - 이러한 프로그램은 패킷 조작, 삭제, 리다이렉션하여 TC내에서 사용되는 알고리즘을 사용자 정의할 수 있다.

### XDP 프로그램과 차이
- XDP 프로그램 : 네트워크 데이터가 네트워크 스택에 도달하기 전에 동작
- TC 프로그램 : 데이터가 스택에 들어오고 sk_buff 구조체가 설정된 후에 동작
> 따라서 TC에서는 개별 패킷과 그 처리에 대한 미세한 제어가 가능해진다.(대역폭 제한, 우선순위 설정, 복잡한 네트워크 동작 등)

### Ingress, Egress
- TC에서는 인그레스와 이그레스를 구분한다.
  - 인그레스 : 네트워크 인터페이스로부터 들어오는 트래픽
  - 이그레스 : 네트워크 인터페이스로 향하는 트래픽
- eBPF 프로그램은 (어느방향도 가능하지만) 양방향 중 한 방향에만 연결되며 해당 방향의 트래픽에만 영향을 미친다.


### Classifier
- 패킷을 분류하는 기능을 수행하는 TC의 구성 요소 중 하나
- 패킷의 속성에 따라 패킷을 분류하고 이를 기반으로 다음 단계에서 취해야 할 작업을 결정한다.
- 패킷의 출발지, 목적지 ip 주소나 포트 번호 등과 같은 속성에 따라 패킷을분류할 수 있다.

### Action
- `TC_ACT_SHOT` : 커널에게 패킷을 드롭하도록 지시
- `TC_ACT_UNSPEC` : 해당 eBPF 프로그램이 이 패킷에 적용되지 않은 것처럼 동작한다.
- `TC_ACT_OK` : 커널에게 패킷을 다음 레이어로 전달하도록 지시한다.
- `TC_ACT_REDIRECT` : 패킷을 다른 네트워크 장치의 인그레스 또는 이그레스 경로로 보낸다.
---

## Simple Example

### Packet Drop


#### 
```
int tc_drop(struct __sk_buff *skb) {
  bpf_trace_printk("[tc] ingress got packet\n");
  
  return TC_AC_SHOT;
```
- `bpf_trace_printk` 를 이용하여 메시지를 만들고 커널에게 `TC_ACT_SHOT` 을 날려 패킷 드롭을 명령한다.


#### ICMP 패킷만 drop 하는 코드
```
int tc(struct __sk_buff *skb) {
  void *data = (void *)(long)skb->data;
  void *data_end = (void *)(long)skb->data_end;
  
  if (is_icmp_ping_request(data, data_end)) {
    struct iphdr *iph = data + sizeof(struct ethhdr);
    struct icmphdr *icmp = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    bpf_trace_printk("[tc] ICMP request for %x type %x\n", iph->daddr,
                      icmp->type);
    return TC_ACT_SHOT;
   }
  return TC_ACT_OK;
}
```
- `skb` : struct __sk_buff 타입의 포인터로 지정, 네트워크 패킷에 대한 정보를 담고 있는 구조체

1. `skb` 의(sk_buff 구조체의) `data`와 `data_end 포인터를 이용하여 패킷 데이터의 시작과 끝을 지정한다.
   - 캐스팅 하는 이유
   - long : 주로 32비트와 64비트 시스템에서의 호환성을 하기 위함
     - 주로 64비트 시스템에서 64비트 정수를 나타내는 데 사용되며, 포인터 값을 저장하기에 충분한 크기를 가짐
   - (void *) : skb->data 와 skb->data_end 의 원래 타입이 __u32(32비트 정수타입) 이다.
     - 이때 skb_data 와 skb->data 는 주소 값을 가리키는 포인터로 사용되어야 하므로, void * 로 캐스팅 한 것
2. `ìs_icmp_ping_request` 함수를 이용하여 현재 패킷이 ICMP ping 요청 패킷인지 확인한다.
3. ICMP ping 패킷인 경우 패킷 내부의 IP 헤더와 ICMP 헤더에 접근하여 해당 정보를 로그에 출력한다.
   - iph->daddr : 목적지 IP 주소 의미
   - icmp->type : ICMP 패킷의 타입을 의미해 ICMP 메시지 유형을 식별하는 역할을 한다.
4. `TC_ACT_SHOT` 을 반환하여 해당 패킷을 버리도록 지시한다.
5. ICMP ping 요청 패킷이 아닌 경우 `TC_ACK_OK`를 반환하여 다음 레벨의 클래스파이어로 패킷을 전달한다.
![hdr.png](..%2Fimg%2Fhdr.png)

### 특정 패킷을 식별 및 ping 으로 응답하기

```
int tc_pingpong(struct __sk_buff *skb) {
    void *data = (void *)(long)skb->data;
    void *data = (void *)(long)skb->data_end;
    
    if (!is_icmp_ping_request(data, data_end)) {
        return TC_ACK_OK;
    }
    
    struct iphdr *iph = data + sizeof(struct ethhdr);
    struct icmphdr *icmp = data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    
    swap_mac_addresses(skb);
    swap_ip_addresses(skb);
    
    // ICMP 패킷의 타입을 0으로 바꾼다( 0 : ICMP Echo Reply , 8(기존) : ICMP Echo Request)
    update_icmp_type(skb, 8, 0);
    
    // 
    bpf_clone_redirect(skb, skb->ifindex, 0);
    
    return TC_ACT_SHOT;
}
```

- `is_icmp_ping_request()` 함수를 이용하여 패킷을 파싱하고 ICMP 메시지인지와 ping 요청인지 확인한다.
  - is_icmp_ping_request() 와 같은 함수들은 모두 사용자 정의 함수들이다.
- 요청을 응답으로 바꾸기 위해 swap 함수를 이용해 출발지 및 목적지 주소를 스왑한다.
- ICMP 헤더의 type 필드를 변경하여 이를 echo reply 로 바꾼다.
- `bpf_clone_redirect()` : 수신된 인터페이스를 통해 패킷의 클론을 다시 보내는 헬퍼 함수 
  - skb->ifindex : ifindex 는 sk_buff 구조체의 멤버로, 패킷이 수신되거나 전송되는 네트워크 인터페이스의 인덱스를 나타낸다.
  - 이를 통해 네트워크 인터페이스를 식별하고 조작하는데 사용 가능