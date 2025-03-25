#define __TARGET_ARCH_x86
#define MSG_SIZE 128

#include "vmlinux.h"

/* Tracing structure definitions */
struct trace_event_raw_sys_enter {
    __u64 pad;         /* Padding might vary based on architecture */
    __u32 id;          /* Syscall ID */
    __u64 args[6];     /* Syscall arguments */
};

#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>

// #include <linux/types.h> generates 'asm/types.h' file not found

// Define the structure for the event data that will be sent to user space
struct event {
    __u32 pid;          // Process ID
    char comm[16];      // Command name (process name)
    char filename[256]; // Filename being deleted
};

// Define a BPF map to send events to user space
struct {
    __uint(type, BPF_MAP_TYPE_PERF_EVENT_ARRAY); // Type of BPF map
    __uint(max_entries, 1024);                   // Maximum number of entries in the map
    __type(key, int);                            // Type of the key
    __type(value, int);                          // Type of the value
} events SEC(".maps");                            // Place the map in the "maps" section

SEC("tracepoint/syscalls/sys_enter_unlinkat")
int trace_unlinkat(struct trace_event_raw_sys_enter* ctx) {
    struct event evt = {}; // Initialize an event structure

    evt.pid = bpf_get_current_pid_tgid() >> 32;

    bpf_get_current_comm(&evt.comm, sizeof(evt.comm));

    bpf_probe_read_user_str(&evt.filename, sizeof(evt.filename), (void *)(ctx->args[1]));

    // Output the event to user space
    bpf_perf_event_output(ctx, &events, BPF_F_CURRENT_CPU, &evt, sizeof(evt));

    return 0; //success
}

// Define the license for the eBPF program
char _license[] SEC("license") = "GPL";