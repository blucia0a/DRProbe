#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>

void sigtrap(int nr)
{
	printf("Trap! (%d)\n", nr);
}

int main(int argc, char *argv[])
{
	int dr;
	uint64_t val;
	int fd, n;
	char line[64];
	char buffer[32];
	char cmd;
	char *tok1, *tok2, *tok3;
	char *tok;

	signal(SIGTRAP, sigtrap);
	
	printf("&cmd: %p\n", &cmd);
	printf("ll:   %d\n", sizeof(long long));

	while (1) {
		fd = -1, dr = -1;
		printf("> ");
		fgets(line, sizeof line - 1, stdin);
		line[strlen(line)-1] = '\0';
		
		tok = strtok(line, " ");
		if (tok != NULL) {
			cmd = *tok;

			tok = strtok(NULL, " ");
			if (tok) {
				dr = atoi(tok);
				snprintf(buffer, sizeof buffer, "/proc/dr/dr%d", dr);
				fd = open(buffer, O_RDWR);

			} else {
				printf("Missing register number\n");
				continue;
			}

		} else {
			printf("No command.\n");
			continue;
		}

		switch (cmd) {
		case 'w':
			tok = strtok(NULL, " ");
			if (tok) {
				val = strtoll(tok, NULL, 0);
				printf("Writing %ld (%lx) to dr%d\n", val, val, dr);
				if (fd < 0) {
					printf("Oops.\n");
					continue;
				}

				write(fd, &val, sizeof val);

			} else {
				printf("Missing value\n");
				continue;
			}

			/* Fall through */

		case 'r':
			n = read(fd, &val, sizeof val);
			printf("Got %d bytes: %ld\n", n, val);
			break;
		
		default:
			printf("Bad cmd '%c'\n", cmd);
			break;
		}

		if (dr >= 0 && fd >= 0)
			close(fd);
	}

	return 0;
}
