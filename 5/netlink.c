#include "linux/printk.h"
#include <linux/module.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/net_namespace.h>

#define NETLINK_USER 31

#define NETLINK_PORT_ID 0
#define NETLINK_SEQUENCE_NUMBER 0
#define NETLINK_FLAGS 0

struct sock *nl_sk = NULL;

static void hello_nl_recv_msg(struct sk_buff *skb)
{

    struct nlmsghdr *nlh;
    int pid;
    struct sk_buff *skb_out;
    int msg_size;
    char *msg = "Hello from kernel";
    int res;

    pr_info("netlink: entering: %s\n", __FUNCTION__);

    msg_size = strlen(msg);

    nlh = (struct nlmsghdr *)skb->data;
    pr_info("netlink: netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));
    pid = nlh->nlmsg_pid; 

    skb_out = nlmsg_new(msg_size, 0);

    if (!skb_out)
    {

        pr_err("netlink: failed to allocate new skb\n");
        return;

    }
    nlh = nlmsg_put(skb_out, NETLINK_PORT_ID, NETLINK_SEQUENCE_NUMBER, NLMSG_DONE, msg_size, NETLINK_FLAGS);
    NETLINK_CB(skb_out).dst_group = 0;
        strncpy(nlmsg_data(nlh), msg, msg_size);

    res = nlmsg_unicast(nl_sk, skb_out, pid);

    if (res < 0)
        pr_err("netlink: error while sending bak to user\n");
}

struct netlink_kernel_cfg cfg = {
   .groups  = 1,
   .input = hello_nl_recv_msg,
};

static int __init netlink_init(void) {
	pr_info("netlink: loading module \n");

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);

    if (!nl_sk)
    {
        pr_alert("netlink: error creating socket.\n");
        return -1;
    }

	pr_info("netlink: module loaded \n");
    return 0;
}

static void __exit netlink_exit(void) {
	pr_info("netlink: stopping module\n");
    netlink_kernel_release(nl_sk);
	pr_info("netlink: module stopped\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Artemiy Dement'ev");
MODULE_DESCRIPTION("Simple module utilizing netlink");

module_init(netlink_init);
module_exit(netlink_exit);
