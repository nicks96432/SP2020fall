#! /bin/bash

function cleanup() {
	for ((i = 0; i <= n_host; ++i)); do
		exec {fifofds[$i]}>&- #關FIFO
		rm "./fifo_$i.tmp"    #刪FIFO
	done
}

trap cleanup SIGTERM

# 確認參數正確
if [[ $# -ne 2 ]]; then
	echo "usage:bash $0 [n_host] [n_player]"
	exit 1
fi

# 宣告變數
n_host=$1      # host數
n_player=$2    # player數
hostkeys=(123) # 用來記錄每個host的key
hosttate=()    # 用來記錄每個host的狀態
# fifofds    # 用來記錄fifo的fd
score=()     # 用來記錄每個player的成績
rankscore=() # 1~8名可獲得的成績
IDLE=0       # 定義空閒的host的state
WORKING=1    # 定義正在使用的host的state
FALSE=0
TRUE=1

# 建立並打開0~(n_host - 1)的FIFO
for ((i = 0; i <= $n_host; ++i)); do
	mkfifo "./fifo_$i.tmp"
	exec {fifofds[$i]}<>"fifo_$i.tmp"
done

# 歸零成績
for ((i = 0; i <= n_player; ++i)); do
	score[i]=0
done

# 建立各個名次所能獲得的分數表
for ((i = 0; i <= 8; ++i)); do
	rankscore[i]=$((8 - $i))
done

# 啟動hosts
for ((i = 1; i <= $n_host; ++i)); do
	hostKey=$(((($RANDOM & 1) << 15) + $RANDOM))
	hoststate+=($IDLE)
	hostkeys+=($hostKey)
	./host $i $hostKey 0 &
done

# 偽遞迴
for ((p1 = 1; p1 <= $n_player; ++p1)); do
	for ((p2 = p1 + 1; p2 <= $n_player; ++p2)); do
		for ((p3 = p2 + 1; p3 <= $n_player; ++p3)); do
			for ((p4 = p3 + 1; p4 <= $n_player; ++p4)); do
				for ((p5 = p4 + 1; p5 <= $n_player; ++p5)); do
					for ((p6 = p5 + 1; p6 <= $n_player; ++p6)); do
						for ((p7 = p6 + 1; p7 <= $n_player; ++p7)); do
							for ((p8 = p7 + 1; p8 <= $n_player; ++p8)); do
								player=($p1 $p2 $p3 $p4 $p5 $p6 $p7 $p8) # 遞迴結果
								findidle=$FALSE
								for choose in $(seq 1 $n_host); do
									if [[ hoststate[$choose] -eq $IDLE ]]; then
										hoststate[$choose]=$WORKING
										echo ${player[@]} >"fifo_$choose.tmp"
										findidle=$TRUE
										break
									fi
								done
								# 找空閒的host
								if [[ findidle -eq $FALSE ]]; then
									read line <"fifo_0.tmp"
									for ((i = 1; i <= 8; ++i)); do
										read -a a <"fifo_0.tmp"
										((score[a[0]] += rankscore[a[1]]))
									done
									for ((i = 1; i <= n_host; ++i)); do
										if [[ $line -eq hostkeys[i] ]]; then
											echo ${player[@]} >"fifo_$i.tmp"
											break
										fi
									done
								fi
							done
						done
					done
				done
			done
		done
	done
done
# 讀完最後的部分
for ((i = 1; i <= n_host; ++i)); do
	if [[ hoststate[$i] -eq $WORKING ]]; then
		hoststate[$i]=$IDLE
		read line <fifo_0.tmp
		for ((j = 1; j <= 8; ++j)); do
			read -a a <fifo_0.tmp
			((score[${a[0]}] += rankscore[${a[1]}]))
		done
	fi
	echo "-1 -1 -1 -1 -1 -1 -1 -1" >"fifo_$i.tmp"
done

# 等所有host跑完
wait

# 刪掉所有FIFO
cleanup

# 印答案
for ((i = 1; i <= n_player; ++i)); do
	echo $i ${score[$i]}
done
