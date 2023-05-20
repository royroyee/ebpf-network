#include <linux/bpf.h>
#include <linux/pkt_cls.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <arpa/inet.h>
#include <bpf/bpf_helpers.h>
#include <linux/if_arp.h>

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