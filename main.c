#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vma.h"
#include <errno.h>

#define NMAX 100
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

int permissions_calculate(char *p)
{
	if (strcmp(p, "PROT_NONE") == 0)
		return 0;

	if (strcmp(p, "PROT_WRITE") == 0)
		return 2;

	if (strcmp(p, "PROT_READ") == 0)
		return 4;

	if (strcmp(p, "PROT_EXEC") == 0)
		return 1;

	return -1;
}

int main(void)
{
	char comanda1[NMAX];
	arena_t *arena;
	while (1) {
		scanf("%s", comanda1);
		if (strcmp(comanda1, "ALLOC_ARENA") == 0) {
			uint64_t size;
			scanf("%ld", &size);
			arena = alloc_arena(size);

		} else if (strcmp(comanda1, "DEALLOC_ARENA") == 0) {
			dealloc_arena(arena);
			break;
		} else if (strcmp(comanda1, "ALLOC_BLOCK") == 0) {
			uint64_t address, size;
			scanf("%ld", &address);
			scanf("%ld", &size);
			alloc_block(arena, address, size);

		} else if (strcmp(comanda1, "FREE_BLOCK") == 0) {
			uint64_t size;
			scanf("%ld", &size);
			free_block(arena, size);

		} else if (strcmp(comanda1, "READ") == 0) {
			uint64_t address, size;
			scanf("%ld", &address);
			scanf("%ld", &size);
			read(arena, address, size);

		} else if (strcmp(comanda1, "WRITE") == 0) {
			uint64_t address, size, size2 = 0;
			void *data, *data2;
			scanf("%ld", &address);
			scanf("%ld", &size);
			getchar();
			data = malloc(size + 1);
			DIE(!data, "Alocare esuata");
			fgets(data, size + 1, stdin);
			size2 = strlen(data);
			size2 = size - size2;
			while (size2 > 0) {
				data2 = malloc(size2 + 1);
				DIE(!data2, "Alocare esuata");
				fgets(data2, size2 + 1, stdin);
				size2 -= strlen(data2);
				strcat(data, data2);
				free(data2);
			}
			write(arena, address, size, data);
			free(data);

		} else if (strcmp(comanda1, "PMAP") == 0) {
			pmap(arena);

		} else if (strcmp(comanda1, "MPROTECT") == 0) {
			uint64_t address = 0;
			char *permission = NULL;
			scanf("%ld", &address);
			permission = malloc(NMAX * sizeof(char));
			DIE(!permission, "Alocare esuata");
			int perm = 0;
			fgets(permission, NMAX, stdin);
			char *p;
			p = strtok(permission, "\n ");
			perm += permissions_calculate(p);
			p = strtok(NULL, "\n ");
			while (p) {
				p = strtok(NULL, "\n ");
				perm += permissions_calculate(p);
				p = strtok(NULL, "\n ");
			}
			mprotect(arena, address, (int8_t *)(&perm));
			free(permission);

		} else {
			printf("Invalid command. Please try again.\n");
		}
	}
	return 0;
}
