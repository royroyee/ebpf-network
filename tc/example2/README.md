

## tc-bpf example : Redirection

### Code
```
__attribute__((section("ingress"), used))
int forward_arp_broadcast(struct __sk_buff *skb) {

    struct arphdr *arp;

    const int l3_off = ETH_HLEN;                      // IP header offset
    const int l4_off = l3_off + sizeof(struct iphdr); // L4 header offset

    void *data = (void*)(long)skb->data;
    void *data_end = (void*)(long)skb->data_end;


    if (data_end < data + l4_off)
        return TC_ACT_OK;


    struct ethhdr *eth = data;
    arp = data + sizeof(*eth);


      // Check if the packet is an ARP broadcast request
    if (eth->h_proto == htons(ETH_P_ARP)) {

        struct arphdr *arp = data + l3_off;

        if (ntohs(arp->ar_op) == ARPOP_REQUEST) {

        // Redirect the packet to the desired interface index
        bpf_clone_redirect(skb, 13, 0);  // Replace '13' with the desired interface index

        return TC_ACT_SHOT;  // Drop the original packet
            }
        }

  // Return TC_ACT_UNSPEC for other packets to be processed by the next classifier
    return TC_ACT_UNSPEC;
}
```


- `bpf_clone_redirect()` : 패킷의 복제본을 생성하고, 리다이렉션 시킬 인터페이스 인덱스를 입력하여 리다이렉션 시킨다. 
  - `skb` : 리다이렉트할 패킷에 대한 포인터
  - `index` : 리다이렉트할 대상 인터페이스 인덱스
  - `flags` : 리다이렉트 동작에 대한 플래그. 0 : 기본 동작 유지 (일반적으로 0 사용)
  - 복제본을 생성하여 리다이렉션 시키기 때문에, 원본 패킷에 대한 처리가 필요하다. (여기선 `TC_ACT_SHOT` 으로 원본 패킷 Drop)




### bridge interface index 확인하기
```cgo
cat /sys/class/net/br0/ifindex
```
