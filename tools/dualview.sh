killall screen
tmux -2 new-session -d -s ig
tmux bind-key \& kill-window
tmux bind-key x kill-window
tmux split-window -h
tmux select-pane -t 0
tmux send-keys "screen /dev/ttyUSB0 115200" Enter
tmux select-pane -t 1
tmux send-keys "screen /dev/ttyUSB1 115200" Enter
tmux -2 attach-session -t ig
killall screen
