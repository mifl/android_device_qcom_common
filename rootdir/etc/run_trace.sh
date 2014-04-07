echo "Trace setup started"
mount -t debugfs none /sys/kernel/debug
echo nop > /sys/kernel/debug/tracing/current_tracer
echo sched:* > /sys/kernel/debug/tracing/set_event
sleep 1
echo 10000 > /sys/kernel/debug/tracing/buffer_size_kb
echo 1 > /sys/kernel/debug/tracing/trace
echo 1 > /sys/kernel/debug/tracing/tracing_on

echo "Tracing setup Completed"

