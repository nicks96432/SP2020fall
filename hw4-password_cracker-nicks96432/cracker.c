#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <openssl/md5.h>

#define MAXSTRLEN 1024

#define err_sys(err, args...)         \
	do                                \
	{                                 \
		fprintf(stderr, err, ##args); \
		exit(1);                      \
	} while (0);

static int N;
static FILE *outfile;
static const char *goal;
static char ***passwordStrings;
static char **nowStrings, **md5results;
static unsigned char **md5hextmp;

void getMD5string(const char *str, size_t len, char *result, int M)
{
	if (len < 0)
		len = strlen(str);
	MD5(str, len, md5hextmp[M]);
	for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
		sprintf(result + i * 2, "%02x", md5hextmp[M][i]);
}

static inline int countSame(const char *a, const char *b)
{
	int result = 0;
	for (; *a == *b && *a != '\0' && *b != '\0'; ++a, ++b)
		++result;
	return result;
}

void findTreasureInit(char *prefix, int nowLen, int maxLen, int *count, int M)
{
	if (nowLen == maxLen)
	{
		getMD5string(prefix, nowLen, md5results[*count], *count);
		if (countSame(md5results[*count], goal) >= 1)
			strcpy(passwordStrings[(*count)++][1], prefix);
		return;
	}
	for (prefix[nowLen] = ' '; prefix[nowLen] <= '~'; ++prefix[nowLen])
	{
		findTreasureInit(prefix, nowLen + 1, maxLen, count, M);
		if (*count == M)
			return;
	}
	prefix[nowLen] = '\0';
}

bool findTreasure(char *prefix, int nowLen, int maxLen, int M, int N)
{
	if (nowLen == maxLen)
	{
		getMD5string(prefix, nowLen, md5results[M], M);
		if (countSame(md5results[M], goal) >= N)
		{
			strcpy(passwordStrings[M][N], prefix);
			return true;
		}
		return false;
	}
	for (prefix[nowLen] = ' '; prefix[nowLen] <= '~'; ++prefix[nowLen])
	{
		getMD5string(prefix, nowLen + 1, md5results[M], M);
		if (countSame(md5results[M], goal) >= N)
		{
			strcpy(passwordStrings[M][N], prefix);
			return true;
		}
	}
	for (prefix[nowLen] = ' '; prefix[nowLen] <= '~'; ++prefix[nowLen])
		if (findTreasure(prefix, nowLen + 1, maxLen, M, N))
		{
			prefix[nowLen] = '\0';
			return true;
		}
	prefix[nowLen] = '\0';
	return false;
}

void *threadRoutine(void *arg)
{
	int M = *(int *)arg;
	for (int i = 2; i <= N; ++i)
	{
		strcpy(nowStrings[M], passwordStrings[M][i - 1]);
		nowStrings[M][strlen(nowStrings[M])] = '\0';
		getMD5string(nowStrings[M], strlen(nowStrings[M]), md5results[M], M);
		size_t nowLen = strlen(nowStrings[M]);
		for (int j = nowLen + 1; j < MAXSTRLEN; ++j)
			if (findTreasure(nowStrings[M], nowLen, j, M, i))
				break;
	}
	pthread_exit(NULL);
}

int main(int argc, const char *argv[])
{
	if (argc < 6)
		err_sys("usage: %s [prefix] [Goal] [N] [M] [outfile]\n", argv[0]);

	/* init */
	const char *const prefix = argv[1];
	goal = argv[2];
	char *test;
	errno = 0;
	N = strtol(argv[3], &test, 10);
	if (*test != '\0' || errno == ERANGE)
		err_sys("invalid N\n");
	errno = 0;
	const long M = strtol(argv[4], &test, 10);
	if (*test != '\0' || errno == ERANGE)
		err_sys("invalid M\n");
	const char *const outfileName = argv[5];
	outfile = fopen(outfileName, "w");
	if (outfile == NULL)
		err_sys("fopen error\n");

	/* 把每個M的第一個password string找出來 */
	int *threadsArgs = (int *)malloc(sizeof(int) * M);
	assert(threadsArgs != NULL);
	passwordStrings = (char ***)malloc(sizeof(char **) * M);
	assert(passwordStrings != NULL);
	nowStrings = (char **)malloc(sizeof(char *) * M);
	assert(nowStrings != NULL);
	md5results = (char **)malloc(sizeof(char *) * M);
	assert(md5results != NULL);
	md5hextmp = (unsigned char **)malloc(sizeof(unsigned char *) * M);
	assert(md5hextmp != NULL);
	for (int i = 0; i < M; ++i)
	{
		passwordStrings[i] = (char **)malloc(sizeof(char *) * (N + 1));
		assert(passwordStrings[i] != NULL);
		nowStrings[i] = (char *)malloc(sizeof(char) * MAXSTRLEN);
		assert(nowStrings[i] != NULL);
		md5results[i] = (char *)malloc(sizeof(char) * MAXSTRLEN);
		assert(md5results[i] != NULL);
		md5hextmp[i] = (unsigned char *)malloc(sizeof(unsigned char) * MD5_DIGEST_LENGTH);
		assert(md5hextmp[i] != NULL);
		for (int j = 1; j <= N; ++j)
		{
			passwordStrings[i][j] = (char *)malloc(sizeof(char) * MAXSTRLEN);
			assert(passwordStrings[i][j] != NULL);
		}
		threadsArgs[i] = i;
	}
	strcpy(nowStrings[0], prefix);
	size_t prefixLen = strlen(prefix);
	int countString = 0;
	for (int i = 1; countString < M; ++i)
		findTreasureInit(nowStrings[0], prefixLen, prefixLen + i, &countString, M);

	/* create M個thread找完剩下的 */
	pthread_t threads[M];
	for (int i = 0; i < M; ++i)
		pthread_create(&threads[i], NULL, threadRoutine, &threadsArgs[i]);

	for (int i = 0; i < M; ++i)
	{
		pthread_join(threads[i], NULL);
		for (int j = 1; j <= N; ++j)
		{
			fprintf(outfile, "%s\n", passwordStrings[i][j]);
			free(passwordStrings[i][j]);
		}
		fprintf(outfile, "===\n");
		fflush(outfile);
		free(md5results[i]);
		free(md5hextmp[i]);
		free(nowStrings[i]);
		free(passwordStrings[i]);
	}
	free(passwordStrings);
	free(threadsArgs);
	free(nowStrings);
	free(md5hextmp);
	free(md5results);
	fclose(outfile);
	pthread_exit(NULL);
}

#undef MAXSTRLEN
#undef err_sys