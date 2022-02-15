#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#define err_sys(e) \
	do             \
	{              \
		perror(e); \
		abort();   \
	} while (0)
typedef struct
{
	int first, second;
} pair;
int sortScore(const void *x, const void *y)
{
	const pair a = *(const pair *)x, b = *(const pair *)y;
	if (a.second > b.second)
		return -1;
	else if (a.second == b.second)
		return 0;
	else
		return 1;
}
int sortId(const void *x, const void *y)
{
	const pair a = *(const pair *)x, b = *(const pair *)y;
	if (a.first > b.first)
		return 1;
	else if (a.first == b.first)
		return 0;
	else
		return -1;
}
void generateChildren(char const *programName, char const *host_id, char const *key,
					  int depth, int player1, int player2,
					  int leftPipeToChild[2], int leftPipeToParent[2],
					  int rightPipeToChild[2], int rightPipeToParent[2])
{
	pid_t childLeft;			  // 左邊小孩的pid
	if ((childLeft = fork()) < 0) // fork error
		err_sys("childLeft fork error");
	else if (childLeft == 0) // 左邊的小孩
	{
		// 不用關掉stdin和stdout，dup2會默默地關掉
		if (depth < 2 && dup2(leftPipeToChild[0], STDIN_FILENO) < 0) // 小孩從這裡讀
			err_sys("childLeft dup2 leftPipeToChild error");
		if (dup2(leftPipeToParent[1], STDOUT_FILENO) < 0) // 小孩從這裡寫
			err_sys("childLeft dup2 leftPipeToParent error");
		if (depth < 2 && close(leftPipeToChild[0]) < 0)
			err_sys("childLeft close leftPipeToChild[0] error");
		if (depth < 2 && close(leftPipeToChild[1]) < 0)
			err_sys("childLeft close leftPipeToChild[1] error");
		if (close(leftPipeToParent[0]) < 0)
			err_sys("childLeft close leftPipeToParent[0] error");
		if (close(leftPipeToParent[1]) < 0)
			err_sys("childLeft close leftPipeToParent[1] error");
		// 關掉右邊小孩的pipe，左邊用不到
		if (depth < 2 && close(rightPipeToChild[0]) < 0)
			err_sys("childLeft close rightPipeToChild[0] error");
		if (depth < 2 && close(rightPipeToChild[1]) < 0)
			err_sys("childLeft close rightPipeToChild[1] error");
		if (close(rightPipeToParent[0]) < 0)
			err_sys("childLeft close rightPipeToParent[0] error");
		if (close(rightPipeToParent[1]) < 0)
			err_sys("childLeft close rightPipeToParent[1] error");
		if (depth < 2) // 在depth < 2時要fork host
		{
			char depthStr[10] = {}; // 把depth變成字串
			sprintf(depthStr, "%d", depth + 1);
			if (execl(programName, programName, host_id, key, depthStr, NULL) < 0)
				err_sys("childLeft execl host error");
		}
		else if (depth == 2) // 在depth == 2時要exec player [player1]
		{
			char playerstr[10] = {}; // 把player1變成字串
			sprintf(playerstr, "%d", player1);
			if (execl(programName, programName, playerstr, NULL) < 0)
				err_sys("childLeft execl player error");
		}
	}
	pid_t childRight;			   // 右邊小孩的pid
	if ((childRight = fork()) < 0) // fork error
		err_sys("fork childRight error");
	else if (childRight == 0) // 右邊的小孩
	{
		// 不用關掉stdin和stdout，dup2會默默地關掉
		if (depth < 2 && dup2(rightPipeToChild[0], STDIN_FILENO) < 0) // 小孩從這裡讀
			err_sys("childRight dup2 rightPipeToChild error");
		if (dup2(rightPipeToParent[1], STDOUT_FILENO) < 0) // 小孩從這裡寫
			err_sys("childRight dup2 rightPipeToParent error");
		if (depth < 2 && close(rightPipeToChild[0]) < 0)
			err_sys("childRight close rightPipeToChild[0] error");
		if (depth < 2 && close(rightPipeToChild[1]) < 0)
			err_sys("childRight close rightPipeToChild[1] error");
		if (close(rightPipeToParent[0]) < 0)
			err_sys("childRight close rightPipeToParent[0] error");
		if (close(rightPipeToParent[1]) < 0)
			err_sys("childRight close rightPipeToParent[1] error");
		// 關掉左邊小孩的pipe，右邊用不到
		if (depth < 2 && close(leftPipeToChild[0]) < 0)
			err_sys("childRight close leftPipeToChild[0] error");
		if (depth < 2 && close(leftPipeToChild[1]) < 0)
			err_sys("childRight close leftPipeToChild[1] error");
		if (close(leftPipeToParent[0]) < 0)
			err_sys("childRight close leftPipeToParent[0] error");
		if (close(leftPipeToParent[1]) < 0)
			err_sys("childRight close leftPipeToParent[1] error");
		if (depth < 2) // 在depth < 2時要fork host
		{
			char depthStr[5] = {}; // 把depth變成字串
			sprintf(depthStr, "%d", depth + 1);
			if (execl(programName, programName, host_id, key, depthStr, NULL) < 0)
				err_sys("execl childRight error");
		}
		else if (depth == 2)
		{
			char playerstr[10] = {}; // 把player2變成字串
			sprintf(playerstr, "%d", player2);
			if (execl(programName, programName, playerstr, NULL) < 0)
				err_sys("childLeft execl player error");
		}
	}
}
int main(int argc, char const *argv[])
{
	// 確認引數格式正確 ./host [host_id] [key] [depth]
	if (argc < 4)
		err_sys("usage: ./host [host_id] [key] [depth]");
	/* TODO: 流程
	 *  如果depth == 0 (root host)，要打開FIFO
	 *	除了depth == 2 (leaf host)，其他都要fork兩個host
	 *  上層和下層的host之間要用pipe雙向溝通，所以pipe要開兩個
	 *  要讓下一層從stdin讀資料，從stdout寫資料，所以要先用dup2轉向
	 */
	// 把參數轉成數字
	int depth = atoi(argv[3]);
	// 給左邊小孩的pipe，注意pipe是左邊讀右邊寫
	int leftPipeToChild[2], leftPipeToParent[2];
	// 給右邊小孩的pipe
	int rightPipeToChild[2], rightPipeToParent[2];
	FILE *leftPipeToChildFILE, *leftPipeToParentFILE;
	FILE *rightPipeToChildFILE, *rightPipeToParentFILE;
	// 讀左邊小孩：leftPipeToPaerent[0]，寫給左邊小孩：leftPipeToChild[1]
	// 讀右邊小孩：rightPipeToPaerent[0]，寫給右邊小孩：rightPipeToChild[1]
	if (pipe(leftPipeToParent) < 0) // 往上寫的pipe
		err_sys("leftPipeToParent pipe error");
	if (pipe(rightPipeToParent) < 0) // 往上寫的pipe
		err_sys("rightPipeToParent pipe error");
	if ((leftPipeToParentFILE = fdopen(leftPipeToParent[0], "r")) == NULL)
		err_sys("fdopen leftPipeToParent[0] error");
	if ((rightPipeToParentFILE = fdopen(rightPipeToParent[0], "r")) == NULL)
		err_sys("fdopen rightPipeToParent[0] error");
	if (depth < 2)
	{
		if (pipe(leftPipeToChild) < 0) // 往下寫的pipe
			err_sys("leftPipeToChild pipe error");
		if (pipe(rightPipeToChild) < 0) // 往下寫的pipe
			err_sys("rightPipeToChild pipe error");
		if ((leftPipeToChildFILE = fdopen(leftPipeToChild[1], "w")) == NULL)
			err_sys("fdopen leftPipeToChild[1] error");
		if ((rightPipeToChildFILE = fdopen(rightPipeToChild[1], "w")) == NULL)
			err_sys("fdopen rightPipeToChild[1] error");
		generateChildren(argv[0], argv[1], argv[2], depth, 0, 0,
						 leftPipeToChild, leftPipeToParent, rightPipeToChild, rightPipeToParent);
		// 關掉用不到的fd，depth == 2時要留著，因為fork新的player要用到
		if (close(leftPipeToChild[0]) < 0)
			err_sys("parent close leftPipeToChild[0] error");
		if (close(leftPipeToParent[1]) < 0)
			err_sys("parent close leftPipeToParent[1] error");
		if (close(rightPipeToChild[0]) < 0)
			err_sys("parent close rightPipeToChild[0] error");
		if (close(rightPipeToParent[1]) < 0)
			err_sys("parent close rightPipeToParent[1] error");
	}
	if (depth == 0) // root host
	{
		// root host要打開的兩個FIFO的fd
		FILE *fifo_0, *fifo_id;
		// root host要打開的兩個FIFO的名字
		char fifo_0_name[20] = {}, fifo_id_name[20] = {};
		sprintf(fifo_0_name, "fifo_0.tmp");
		sprintf(fifo_id_name, "fifo_%s.tmp", argv[1]);
		// 打開兩個FIFO
		if ((fifo_0 = fopen(fifo_0_name, "r+")) < 0)
			err_sys("fopen fifo_0.tmp error");
		if ((fifo_id = fopen(fifo_id_name, "r")) < 0)
			err_sys("fopen fifo_${host_id}.tmp error");
		// 從fifo_${host_id}.tmp讀資料
		int players[8], playerL, playerR, bidL, bidR;
		while (1)
		{
			for (int i = 0; i < 8; ++i)
				fscanf(fifo_id, "%d", &players[i]);
			fprintf(leftPipeToChildFILE, "%d %d %d %d\n",
					players[0], players[1], players[2], players[3]);
			fflush(leftPipeToChildFILE);
			fprintf(rightPipeToChildFILE, "%d %d %d %d\n",
					players[4], players[5], players[6], players[7]);
			fflush(rightPipeToChildFILE);
			if (players[0] == -1 && players[1] == -1 && players[2] == -1 && players[3] == -1 &&
				players[4] == -1 && players[5] == -1 && players[6] == -1 && players[7] == -1)
				break;
			pair playerSort[13] = {}; // first: player_id, second: score
			for (int i = 0; i < 13; ++i)
				playerSort[i].second = 0x80000000;
			for (int i = 0; i < 8; ++i)
			{
				playerSort[players[i]].first = players[i];
				playerSort[players[i]].second = 0;
			}
			for (int i = 0; i < 10; ++i)
			{
				fscanf(leftPipeToParentFILE, "%d%d", &playerL, &bidL);
				fscanf(rightPipeToParentFILE, "%d%d", &playerR, &bidR);
				if (bidL > bidR)
					++playerSort[playerL].second;
				else
					++playerSort[playerR].second;
			}
			qsort(playerSort, 13, sizeof(pair), sortScore);
			int nowRank = 1, ranks[8], prev = -1;
			for (int i = 0; i < 8; ++i)
			{
				if (playerSort[i].second == prev)
				{
					prev = playerSort[i].second;
					playerSort[i].second = nowRank;
				}
				else
				{
					prev = playerSort[i].second;
					playerSort[i].second = (nowRank = i + 1);
				}
			}
			qsort(playerSort, 8, sizeof(pair), sortId);
			fprintf(fifo_0, "%s\n", argv[2]);
			for (int i = 0; i < 8; ++i)
				fprintf(fifo_0, "%d %d\n", playerSort[i].first, playerSort[i].second);
			fflush(fifo_0);
		}
		wait(NULL);
		wait(NULL);
		if (fclose(fifo_0) == EOF)
			err_sys("fclose fifo_0.tmp error");
		if (fclose(fifo_id) == EOF)
			err_sys("fclose fifo_${host_id}.tmp error");
	}
	else if (depth == 1)
	{
		int players[4], playerL, playerR, bidL, bidR;
		while (1)
		{
			for (int i = 0; i < 4; ++i)
				scanf("%d", &players[i]);
			fprintf(leftPipeToChildFILE, "%d %d\n", players[0], players[1]);
			fflush(leftPipeToChildFILE);
			fprintf(rightPipeToChildFILE, "%d %d\n", players[2], players[3]);
			fflush(rightPipeToChildFILE);
			if (players[0] == -1 && players[1] == -1 && players[2] == -1 && players[3] == -1)
				break;
			for (int i = 0; i < 10; ++i)
			{
				fscanf(leftPipeToParentFILE, "%d%d", &playerL, &bidL);
				fscanf(rightPipeToParentFILE, "%d%d", &playerR, &bidR);
				if (bidL > bidR)
					printf("%d %d\n", playerL, bidL);
				else
					printf("%d %d\n", playerR, bidR);
				fflush(stdout);
			}
		}
		wait(NULL);
		wait(NULL);
	}
	else if (depth == 2)
	{
		int players[2], playerL, playerR, bidL, bidR;
		while (1)
		{
			scanf("%d%d", &players[0], &players[1]);
			if (players[0] == -1 && players[1] == -1)
				break;
			generateChildren("./player", NULL, NULL, 2, players[0], players[1],
							 leftPipeToChild, leftPipeToParent, rightPipeToChild, rightPipeToParent);
			wait(NULL);
			wait(NULL);
			for (int i = 0; i < 10; ++i)
			{
				fscanf(leftPipeToParentFILE, "%d%d", &playerL, &bidL);
				fscanf(rightPipeToParentFILE, "%d%d", &playerR, &bidR);
				if (bidL > bidR)
					printf("%d %d\n", playerL, bidL);
				else
					printf("%d %d\n", playerR, bidR);
				fflush(stdout);
			}
		}
	}
	else if (depth > 2)
		err_sys("depth > 2");
	if (fclose(leftPipeToChildFILE) == EOF)
		err_sys("fclose leftPipe leftToChildFILE error");
	if (fclose(leftPipeToParentFILE) == EOF)
		err_sys("fclose leftPipe leftToParentFILE error");
	if (fclose(rightPipeToChildFILE) == EOF)
		err_sys("fclose leftPipe rightToChildFILE error");
	if (fclose(rightPipeToParentFILE) == EOF)
		err_sys("fclose leftPipe rightToParentFILE error");
	return 0;
}
