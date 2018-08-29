#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <crypt.h>

#define MAX_SEED_LEN 16 

void permute(char *in, int len, int pos);
void generate_replacement_table();
void encrypt_me(const char *s);
int split_target(const char *s);


char *trailers[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9", "123", "!", "?", "$", "#", "" };
char *rep_table[128]; /* only ascii 0x00 - 0x7f */
char *target;
char *salt;
char *seed;
int append_suffix;

void usage(const char *argv_0)
{
	printf("Usage: %s -t <target> seed\n", argv_0);
	printf("  -t       Target hash from /etc/shadow. Program stops if encryption of permuted value matches.\n");
	printf("  seed     The seed string to be modified and run through crypt.\n");

	exit(1);
}

int main(int argc, char **argv)
{
	int opt;

	while ((opt = getopt(argc, argv, "t:h")) != -1) {
		switch (opt) {
		case 't':
			if (!split_target(optarg)) {
				fprintf(stderr, "Invalid target: %s\n", optarg);
				exit(1);
			}

			break;

		case 'h':
		default:
			usage(argv[0]);
			break;
		}
	}

	append_suffix = 1;

	if (!target) {
		fprintf(stderr, "A target is required\n");
		usage(argv[0]);
	}

	if (optind < argc) {
		/* this length limit is arbitrary */
		if (strlen(argv[optind]) > MAX_SEED_LEN) {
			fprintf(stderr, "Max seed length is %d\n", MAX_SEED_LEN);
			exit(1);
		}

		/* later might accept a list, only accept one seed at a time for now */
		seed = strdup(argv[optind]);

		if (!seed) {
			perror("strdup");
			exit(1);
		}
	}
	else {
		fprintf(stderr, "A seed string is required.\n");
		usage(argv[0]);
	}

	generate_replacement_table();

	permute(seed, strlen(seed), 0);

	return 0;
}

int only_valid_crypt_chars(const char *s)
{
	for (; *s; s++) {
		if (!isalpha(*s) && !isdigit(*s) && *s != '.' && *s != '/')
			return 0;
	}

	return 1;
}

int split_target(const char *s)
{
	if (*s != '$' || strlen(s) < 8)
		return 0;

	char *buff = strdup(s + 1);

	if (!buff) {
		perror("strdup");
		exit(1);
	}

	char *p = strtok(buff, "$");

	if (!p || !*p)
		return 0;

	if (strcmp(p, "1") && strcmp(p, "2a") && strcmp(p, "5") && strcmp(p, "6")) {
		fprintf(stderr, "Unknown algo: %s\n", s);
		exit(1);
	}

	//printf("algo: %s\n", p);
 
	p = strtok(NULL, "$");

	if (!p || !*p)
		return 0;

	if (!only_valid_crypt_chars(p))
		return 0;

	//printf("salt: %s\n", p);

	p = strtok(NULL, "$");

	if (!p || !*p)
		return 0;

	if (!only_valid_crypt_chars(p))
		return 0;	

	//printf("encrypt: %s\n", p);

	free(buff);

	salt = strdup(s);
	p = strrchr(salt, '$');
	*p = 0;

	target = strdup(s);
			
	return 1;	
}

void encrypt_me(const char *s)
{
	char *result = crypt(s, salt);

	if (!result) {
		perror("crypt");
		exit(1);
	}

	if (!strcmp(result, target)) {
		printf("Success: %s\n", s);
		printf("%s\n", result);
		exit(0);
	}
}

void permute(char *in, int len, int pos)
{
	int i, rlen, rpos;
	char tbuff[MAX_SEED_LEN * 2];

	if (pos >= len) {
		encrypt_me(in);

		if (append_suffix) {
			bzero(tbuff, sizeof(tbuff));
			strcpy(tbuff, in);
			rlen = strlen(tbuff);

			for (i = 0; *trailers[i] != 0; i++) {
				strcpy(tbuff + rlen, trailers[i]);
				encrypt_me(tbuff);
			}
		}
	}
	else {
		permute(in, len, pos + 1);

		rpos = (int) in[pos];

		if (rpos < 0 || rpos > 127)
			return;

		if (islower(rpos)) {
			in[pos] = toupper(rpos);
			permute(in, len, pos + 1);
			in[pos] = (char)rpos;
		}
		else if (isupper(rpos)) {
			in[pos] = tolower(rpos);
			permute(in, len, pos + 1);
			in[pos] = (char)rpos;
		}

		if (rep_table[tolower(rpos)]) {
			rlen = strlen(rep_table[tolower(rpos)]);

			for (i = 0; i < rlen; i++) {
				in[pos] = rep_table[tolower(rpos)][i];
				permute(in, len, pos + 1);
			}

			in[pos] = (char)rpos;
		}
	}
}

/* this is pretty lame */
void generate_replacement_table()
{
	bzero(rep_table, sizeof(rep_table));

	rep_table['e'] = strdup("3");
	rep_table['h'] = strdup("4");
	rep_table['i'] = strdup("1!");
	rep_table['l'] = strdup("1!");
	rep_table['o'] = strdup("0");
	rep_table['s'] = strdup("z5$");
	rep_table['z'] = strdup("s5$");
	rep_table['t'] = strdup("7");
}
