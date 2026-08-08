// SPDX-License-Identifier: GPL-2.0
/*
 * Minimal stub to register BPF_PROG_TYPE_TRACEPOINT without the full
 * tracing infrastructure (CONFIG_FTRACE/CONFIG_BPF_EVENTS).
 *
 * gpuMem.bpf only uses map helpers (lookup/update/delete) which are
 * handled by the core BPF verifier — no tracing-specific helpers needed.
 */

#include <linux/bpf.h>
#include <linux/filter.h>
#include <linux/mutex.h>
#include <linux/rcupdate.h>
#include <linux/string.h>
#include <linux/tracepoint-defs.h>

#define CREATE_TRACE_POINTS
#include <trace/events/gpu_mem.h>

EXPORT_TRACEPOINT_SYMBOL(gpu_mem_total);

const struct bpf_func_proto * __weak bpf_tracing_func_proto(
	enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	return NULL;
}

const struct bpf_func_proto * __weak tracing_prog_func_proto(
	enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	return NULL;
}

static const struct bpf_func_proto *
tp_stub_func_proto(enum bpf_func_id func_id, const struct bpf_prog *prog)
{
	return bpf_base_func_proto(func_id);
}

static bool tp_stub_is_valid_access(int off, int size,
				    enum bpf_access_type type,
				    const struct bpf_prog *prog,
				    struct bpf_insn_access_aux *info)
{
	/* Tracepoint ctx is a pointer to raw args — allow any aligned read */
	if (type != BPF_READ)
		return false;
	if (off < 0 || off + size > 2048)
		return false;
	if (off % size != 0)
		return false;
	return true;
}

const struct bpf_verifier_ops tracepoint_verifier_ops = {
	.get_func_proto  = tp_stub_func_proto,
	.is_valid_access = tp_stub_is_valid_access,
};

const struct bpf_prog_ops tracepoint_prog_ops = {
};

const struct bpf_verifier_ops raw_tracepoint_verifier_ops = {
	.get_func_proto  = tp_stub_func_proto,
	.is_valid_access = tp_stub_is_valid_access,
};

const struct bpf_prog_ops raw_tracepoint_prog_ops = {
};

const struct bpf_verifier_ops raw_tracepoint_writable_verifier_ops = {
	.get_func_proto  = tp_stub_func_proto,
	.is_valid_access = tp_stub_is_valid_access,
};

const struct bpf_prog_ops raw_tracepoint_writable_prog_ops = {
};

const struct bpf_verifier_ops tracing_verifier_ops = {
	.get_func_proto  = tp_stub_func_proto,
	.is_valid_access = tp_stub_is_valid_access,
};

const struct bpf_prog_ops tracing_prog_ops = {
};

/*
 * Virtual raw tracepoints.
 *
 * bpf_raw_tracepoint_open() in kernel/bpf/syscall.c resolves the target
 * tracepoint through bpf_get_raw_tracepoint() and attaches through
 * bpf_probe_register().  With the full tracing stack disabled there are
 * no real tracepoints (CONFIG_TRACEPOINTS off means every TRACE_EVENT is
 * compiled out), so every attach currently dies with -ENOENT and userspace
 * (gpuMem/gpuWork) logs "Failed to attach bpf program".  Harmless, but
 * noisy.
 *
 * Instead we advertise a tiny fixed set of virtual raw tracepoints.
 * Attaching succeeds, the bpf link stays alive and valid (fdinfo, link
 * info and unregister all work), so userspace is happy.  The underlying
 * tracepoint can never fire in this configuration, so the attached
 * program simply never runs and its maps stay at their zeroed defaults.
 */
struct tp_evict {
	struct bpf_raw_event_map	map;
	struct bpf_prog		*prog;
};

static const char tp_name_gpu_mem_total[] = "gpu_mem/gpu_mem_total";
static const char tp_name_gpu_work_period[] = "power/gpu_work_period";

static struct tracepoint tp_gpu_mem_total = {
	.name = tp_name_gpu_mem_total,
};

static struct tracepoint tp_gpu_work_period = {
	.name = tp_name_gpu_work_period,
};

static struct tp_evict tp_evicts[] = {
	[0] = {
		.map = {
			.tp	     = &tp_gpu_mem_total,
			.num_args   = 4,
		},
	},
	[1] = {
		.map = {
			.tp	     = &tp_gpu_work_period,
			.num_args   = 4,
		},
	},
};

static DEFINE_MUTEX(tp_stub_mutex);

struct bpf_raw_event_map *bpf_get_raw_tracepoint(const char *name)
{
	struct tp_evict *ev;
	int i;

	for (i = 0; i < ARRAY_SIZE(tp_evicts); i++) {
		ev = &tp_evicts[i];
		if (!strcmp(ev->map.tp->name, name))
			return &ev->map;
	}
	return NULL;
}

void bpf_put_raw_tracepoint(struct bpf_raw_event_map *map)
{
	/* no module backing for virtual tracepoints */
}

int bpf_probe_register(struct bpf_raw_event_map *map, struct bpf_prog *prog)
{
	struct tp_evict *ev;

	/* mirror bpf_trace.c bounds checks */
	if (prog->aux->max_ctx_offset > map->num_args * sizeof(u64))
		return -EINVAL;
	if (prog->aux->max_tp_access > map->writable_size)
		return -EINVAL;

	mutex_lock(&tp_stub_mutex);
	ev = container_of(map, struct tp_evict, map);
	ev->prog = prog;
	mutex_unlock(&tp_stub_mutex);
	return 0;
}

int bpf_probe_unregister(struct bpf_raw_event_map *map, struct bpf_prog *prog)
{
	struct tp_evict *ev;

	mutex_lock(&tp_stub_mutex);
	ev = container_of(map, struct tp_evict, map);
	if (ev->prog == prog)
		ev->prog = NULL;
	mutex_unlock(&tp_stub_mutex);
	return 0;
}
