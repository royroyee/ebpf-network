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


### ebpf 코드 실행 방법

#### 1. vmlinux.h 이용
- 이유,개념 및 흐름은 이후 다룰 예정

#### 2. 



### 브로드캐스트 패킷 브릿지로 전달하기 (broad.c)
```shell
#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>

#define ETH_ALEN 6

SEC("filter")
int tc_broadcast(struct __sk_buff *skb) {
    struct ethhdr *eth = bpf_hdr_pointer(skb);
    struct iphdr *iph = (struct iphdr *)(eth + 1);

    // Check if the packet is an ARP request
    if (eth->h_proto == htons(ETH_P_ARP)) {
        // Change the destination MAC address to the broadcast address
        unsigned char broadcast[ETH_ALEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        __builtin_memcpy(eth->h_dest, broadcast, ETH_ALEN);

        // Change the destination IP address to the bridge IP address
        iph->daddr = htonl(0x0A0CF401);  // 10.244.2.1

        // Set the egress interface index to the bridge interface
        skb->ifindex = 2;  // Replace with the correct interface index

        // Return TC_ACT_OK to allow the packet to proceed
        return TC_ACT_OK;
    }

    // Return TC_ACT_UNSPEC for other packets to be processed by the next classifier
    return TC_ACT_UNSPEC;
}
```

