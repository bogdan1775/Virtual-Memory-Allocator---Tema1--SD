#include "vma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

// functia aloca arena
arena_t *alloc_arena(const uint64_t size)
{
	//se aloca arena
	arena_t *arena = malloc(sizeof(arena_t));
	DIE(!arena, "Alocare esuata");

	//se aloca lista pentru blockuri
	arena->alloc_list = malloc(sizeof(list_t));
	DIE(!arena->alloc_list, "Alocare esuata");

	arena->arena_size = size;
	arena->alloc_list->head = NULL;
	arena->alloc_list->data_size = sizeof(block_t);
	arena->alloc_list->size = 0;

	return arena;
}

// functia elibereaza memoria pentru un miniblock
void my_free_miniblock(void *node_miniblock)
{
	miniblock_t *miniblock = ((dll_node_t *)node_miniblock)->data;
	if (miniblock->rw_buffer)
		free(miniblock->rw_buffer);

	free(miniblock);
	free(node_miniblock);
}

// functia elibereaza memoria pentru un block
void my_free_block(void *node_block)
{
	block_t *block = ((dll_node_t *)node_block)->data;
	if (block->miniblock_list)
		free(block->miniblock_list);

	free(block);
	free(node_block);
}

// functia elibereaza toata memoria
void dealloc_arena(arena_t *arena)
{
	if (arena->alloc_list->head) {
		dll_node_t *node_block = arena->alloc_list->head;
		dll_node_t *last_block;

		// se elibereaza pentru fiecare block
		for (size_t i = 0; i < arena->alloc_list->size; i++) {
			block_t *block = node_block->data;
			dll_node_t *node_miniblock = block->miniblock_list->head;
			dll_node_t *last_miniblock;

			// se elibereaza pentru fiecare miniblock dintr-un block
			for (size_t j = 0; j < block->miniblock_list->size; j++) {
				last_miniblock = node_miniblock;
				node_miniblock = node_miniblock->next;
				my_free_miniblock(last_miniblock);
			}

			// se trece pe urmatorul nod din lista
			last_block = node_block;
			node_block = node_block->next;
			my_free_block(last_block);
		}
	}

	free(arena->alloc_list);
	free(arena);
}

// functia creeaza un nod nou
dll_node_t *node_create(const void *new_data, unsigned int data_size)
{
	dll_node_t *new;
	new = malloc(sizeof(dll_node_t));
	DIE(!new, "Alocare esuata");

	new->data = malloc(data_size);
	DIE(!new->data, "Alocare esuata");

	// se copiaza data in noul nod
	memcpy(new->data, new_data, data_size);

	return new;
}

// functia returneaza al n-lea nod
dll_node_t *dll_get_nth_node(list_t *list, unsigned int n)
{
	if (n >= list->size)
		n = n - 1;

	dll_node_t *curr = list->head;
	unsigned int nr = 0;

	// se parcurge lista pana la nodul n
	while (nr < n) {
		curr = curr->next;
		nr++;
	}

	return curr;
}

// functia elimina al n-lea nod si intoarce un pointer la nodul eliminat
dll_node_t *dll_remove_nth_node(list_t *list, unsigned int n)
{
	if (!list || list->size == 0)
		return NULL;

	if (n >= list->size)
		n = list->size - 1;

	// eliminare daca este primul nod
	if (n == 0) {
		dll_node_t *curr;
		curr = list->head;

		list->head = list->head->next;
		if (list->head)
			list->head->prev = NULL;

		list->size--;
		return curr;
	}

	dll_node_t *curr, *last;
	unsigned int nr = 0;

	curr = list->head;
	last = list->head;

	// parcurgere pana la al n-lea nod
	while (curr->next && nr < n - 1) {
		curr = curr->next;
		nr++;
	}

	// nodul care trebuie eliminat
	last = curr->next;

	curr->next = last->next;
	if (last->next)
		last->next->prev = curr;

	list->size--;
	return last;
}

// functia adauga un nod pe pozitia n
void dll_add_nth_node(list_t *list, unsigned int n, const void *new_data)
{
	if (!list)
		return;

	// se creeaza nodul
	dll_node_t *new;
	new = node_create(new_data, list->data_size);

	if (n > list->size)
		n = list->size;

	// daca nodul trebuie adaugat pe prima pozitie
	if (list->size == 0 || n == 0) {
		new->next = list->head;
		new->prev = NULL;
		if (list->head)
			list->head->prev = new;

		list->head = new;

		list->size++;

	} else {
		dll_node_t *curr = list->head;
		unsigned int nr = 0;

		//se parcurge pana la pozitia n
		while (curr->next && nr < n - 1) {
			curr = curr->next;
			nr++;
		}

		//se realizeaza legaturile
		new->next = curr->next;
		new->prev = curr;

		if (curr->next)
			curr->next->prev = new;
		curr->next = new;

		list->size++;
	}
}

// se unesc blockurile care sunt vecine
void adiacent_block(arena_t *arena)
{
	list_t *list = arena->alloc_list;
	dll_node_t *curr = list->head, *curr2;

	while (curr->next) {
		curr2 = curr->next;

		int address = ((block_t *)curr->data)->start_address;
		int address2 = ((block_t *)curr2->data)->start_address;

		int size = ((block_t *)curr->data)->size;
		int size2 = ((block_t *)curr2->data)->size;

		if (address + size == address2) {
			block_t *block = curr->data;
			block_t *block2 = curr2->data;

			block->size = size + size2;
			dll_node_t *node = block->miniblock_list->head;
			while (node->next)
				node = node->next;

			// se unesc listele de miniblockuri
			node->next = block2->miniblock_list->head;
			block2->miniblock_list->head->prev = node;

			// actualizare numar de blockuri
			arena->alloc_list->size--;

			// actualizare numar de miniblokuri
			block->miniblock_list->size += block2->miniblock_list->size;

			// se elimina blockul din lista de blockuri
			if (!curr2->next) {
				curr->next = NULL;
			} else {
				curr->next = curr2->next;
				curr2->next->prev = curr;
			}

			//se elibereaza memoria pentru block
			my_free_block(curr2);

			// se intoarce pe blockul precedent
			if (curr->prev)
				curr = curr->prev;

		} else {
			//se trece pe urmatorul block
			curr = curr->next;
		}
	}
}

// functia aloca memorie pentru un block si il adauga in lista de blockuri
void add_nth_block(arena_t *arena, size_t n, const uint64_t address,
				   const uint64_t size)
{
	list_t *list = arena->alloc_list;

	//alocare block
	block_t *data;
	data = malloc(list->data_size);
	DIE(!data, "Alocare esuata");

	//initiere valori pentru block
	data->start_address = address;
	data->size = size;

	data->miniblock_list = malloc(sizeof(list_t));
	DIE(!data->miniblock_list, "Alocare esuata");

	data->miniblock_list->data_size = sizeof(miniblock_t);

	// alocare miniblock si initiere valori
	dll_node_t *new_miniblock;
	new_miniblock = malloc(sizeof(dll_node_t));
	DIE(!new_miniblock, "Alocare esuata");

	new_miniblock->data = malloc(sizeof(miniblock_t));
	DIE(!new_miniblock->data, "Alocare esuata");

	((miniblock_t *)new_miniblock->data)->start_address = address;
	((miniblock_t *)new_miniblock->data)->size = size;
	((miniblock_t *)new_miniblock->data)->perm = 6;
	((miniblock_t *)new_miniblock->data)->rw_buffer = NULL;
	new_miniblock->next = NULL;
	new_miniblock->prev = NULL;

	data->miniblock_list->head = new_miniblock;
	data->miniblock_list->size = 1;

	// adaugare block pe pozitia n
	dll_add_nth_node(arena->alloc_list, n, data);

	//eliberare data
	free(data);
}

// functia aloca un block in interiorul arenei
void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	dll_node_t *curr = arena->alloc_list->head;
	block_t *block;

	// verificare daca poate fi adaugat
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return;
	}

	if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return;
	}

	while (curr) {
		block = curr->data;
		uint64_t address_b = block->start_address;

		if (address_b + block->size > address && address_b < address + size) {
			printf("This zone was already allocated.\n");
			return;
		}
		curr = curr->next;
	}

	// incepere adaugare

	curr = arena->alloc_list->head;
	size_t n = 0;

	while (curr && ((block_t *)curr->data)->start_address < address) {
		n++;
		curr = curr->next;
	}

	// adaugare block
	add_nth_block(arena, n, address, size);

	// unire blockuri daca sunt vecine
	adiacent_block(arena);
}

// se gaseste pozitia miniblockului care incepe cu adresa data
// se returneaza -1 daca niciun miniblock nu incepe cu adresa data
// altfel returneaza pozitia
int find_poz(list_t *list_mb, int n, const uint64_t address)
{
	dll_node_t *node = list_mb->head;

	for (int i = 0; i < n; i++) {
		if (((miniblock_t *)node->data)->start_address == address)
			return i;
		node = node->next;
	}
	return -1;
}

// se calculeaza size-ul pana intr-o pozitie anume
int calculate_size(list_t *list_mb, int poz)
{
	dll_node_t *node = list_mb->head;
	int nr = 0;
	int size = 0;
	while (nr < poz) {
		nr++;
		size += ((miniblock_t *)node->data)->size;
		node = node->next;
	}
	return size;
}

// se verifica conditiile pentru eliberarea unui block
int conditions_free_block(arena_t *arena, const uint64_t address)
{
	dll_node_t *node = arena->alloc_list->head;
	size_t ok = 0;
	while (node) {
		block_t *block = node->data;
		uint64_t address_b = block->start_address;
		if (address >= address_b && address < address_b + block->size) {
			ok = 1;
			break;
		}
		node = node->next;
	}

	if (ok == 0) {
		printf("Invalid address for free.\n");
		return 0;
	}

	return 1;
}

// functia realizeaza operatia de free_block
void free_block(arena_t *arena, const uint64_t address)
{
	if (!conditions_free_block(arena, address))
		return;
	dll_node_t *node = arena->alloc_list->head;
	int poz_block = 0;
	//se pozitioneaza pe blockul din care trebuie sa eliminam
	while (1) {
		block_t *block = node->data;
		uint64_t start_address = block->start_address;
		if (address >= start_address && address < start_address + block->size)
			break;
		poz_block++;
		node = node->next;
	}
	// incepere eliminare
	list_t *list_mb = ((block_t *)node->data)->miniblock_list;
	block_t *block = node->data;
	int poz_minib = find_poz(list_mb, block->miniblock_list->size, address);

	if (poz_minib == -1) {
		printf("Invalid address for free.\n");
		return;
	}
	// eliminare block care are doar un miniblock
	if (block->miniblock_list->size == 1) {
		block->miniblock_list->size = 0;
		my_free_miniblock(block->miniblock_list->head);
		list_t *list = arena->alloc_list;
		dll_node_t *removed_block = dll_remove_nth_node(list, poz_block);
		my_free_block(removed_block);
		return;
	}

	// se elimina miniblockul din lista de miniblockuri
	dll_node_t *removed_minib = dll_remove_nth_node(list_mb, poz_minib);

	// eliminare primul miniblock dintr-un block
	if (poz_minib == 0) {
		block->size -= ((miniblock_t *)removed_minib->data)->size;
		miniblock_t *miniblock = list_mb->head->data;
		block->start_address = miniblock->start_address;
		my_free_miniblock(removed_minib);
		return;
	}

	// eliminare miniblock de la finalul unui block
	if ((size_t)poz_minib == list_mb->size) {
		block->size -= ((miniblock_t *)removed_minib->data)->size;
		my_free_miniblock(removed_minib);
		return;
	}

	// elimininare miniblock din mijloc
	int size_before_poz = calculate_size(list_mb, poz_minib);
	block_t *data;
	// se aloca data unui block nou
	data = malloc(arena->alloc_list->data_size);
	DIE(!data, "Alocare esuata");

	//se initializeaza valorile pentru data
	miniblock_t *removed_mb = removed_minib->next->data;
	data->start_address = removed_mb->start_address;
	data->size = block->size - size_before_poz - removed_mb->size;
	block->size = size_before_poz;

	data->miniblock_list = malloc(sizeof(list_t));
	DIE(!data->miniblock_list, "Alocare esuata");

	data->miniblock_list->data_size = sizeof(miniblock_t);
	data->miniblock_list->head = removed_minib->next;
	removed_minib->prev->next = NULL;
	removed_minib->next->prev = NULL;

	data->miniblock_list->size = block->miniblock_list->size - poz_minib;
	block->miniblock_list->size = poz_minib;

	my_free_miniblock(removed_minib);
	//se adauga blockul in lista de blockuri
	dll_add_nth_node(arena->alloc_list, poz_block + 1, data);
	free(data);
}

// se verifica daca avem permisiuni pentru citire
int check_permissions_read(dll_node_t *node)
{
	while (node) {
		miniblock_t *miniblock = ((miniblock_t *)node->data);
		if (miniblock->perm < 4)
			return 0;
		node = node->next;
	}
	return 1;
}

// functia realizeaza operatia de READ
void read(arena_t *arena, uint64_t address, uint64_t size)
{
	dll_node_t *curr = arena->alloc_list->head;
	block_t *block;
	int ok = 0;
	// verificare daca adresa este valida
	while (curr) {
		block = curr->data;
		uint64_t start_addres = block->start_address;
		if (address >= start_addres && address < start_addres + block->size) {
			ok = 1;
			break;
		}
		curr = curr->next;
	}
	if (ok == 0) {
		printf("Invalid address for read.\n");
		return;
	}

	// pozitionare pe blockul din care trebuie sa citim
	curr = arena->alloc_list->head;
	block = curr->data;
	while (address > block->start_address + block->size) {
		curr = curr->next;
		block = curr->data;
	}
	block = curr->data;

	// verificare daca depaseste zona alocata
	uint64_t size2 = size;
	if (address + size > block->start_address + block->size) {
		size2 = block->start_address + block->size - address;
		printf("Warning: size was bigger than the block size.");
		printf(" Reading %lu characters.\n", size2);
	}

	// incepere citire;
	dll_node_t *node_minib = block->miniblock_list->head;
	miniblock_t *miniblock = node_minib->data;
	size_t read_size = 0, diff2 = 0;
	int diff = 0;
	// verificare permisiuni citire
	if (check_permissions_read(block->miniblock_list->head) == 0) {
		printf("Invalid permissions for read.\n");
		return;
	}
	node_minib = block->miniblock_list->head;

	//se realizeaza citirea
	while (read_size < size2) {
		miniblock = node_minib->data;
		if (address < miniblock->start_address + miniblock->size) {
			diff = (int)address - miniblock->start_address;
			if (diff < 0)
				diff = 0;
			diff2 = miniblock->size;

			if (size2 - read_size < diff2)
				diff2 = size2 - read_size;
			char *sir = calloc(((int)miniblock->size) + 1, sizeof(char));
			if (diff2 != 0 && miniblock->rw_buffer)
				strncpy(sir, ((char *)miniblock->rw_buffer) + diff,
						diff2);
			read_size += diff2;
			printf("%s", sir);
			free(sir);
		}
		node_minib = node_minib->next;
	}
	printf("\n");
}

//verificare permisiuni scriere
int check_permissions_write(dll_node_t *node)
{
	while (node) {
		miniblock_t *miniblock = ((miniblock_t *)node->data);
		if (miniblock->perm < 2)
			return 0;
		if (miniblock->perm >= 4 && miniblock->perm < 6)
			return 0;

		node = node->next;
	}
	return 1;
}

// se realizeaza operatia de WRITE
void write(arena_t *arena, const uint64_t address, const uint64_t size,
		   int8_t *data)
{
	dll_node_t *curr = arena->alloc_list->head;
	block_t *block;
	int ok = 0;
	// verificare daca adresa exista
	while (curr) {
		block = curr->data;
		uint64_t start = block->start_address;
		if (address >= start && address < start + block->size) {
			ok = 1;
			break;
		}
		curr = curr->next;
	}
	if (ok == 0) {
		printf("Invalid address for write.\n");
		return;
	}

	// pozitionare pe blockul unde trebuie sa scriem
	curr = arena->alloc_list->head;
	block = curr->data;
	while (address > block->start_address + block->size) {
		curr = curr->next;
		block = curr->data;
	}
	block = curr->data;

	// verificare daca depaseste zona modificata
	uint64_t size2 = size;
	if (address + size > block->start_address + block->size) {
		size2 = block->start_address + block->size - address;
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %lu characters.\n", size2);
	}

	// initiere valori pentru scriere
	dll_node_t *node_minib = block->miniblock_list->head;
	miniblock_t *miniblock = node_minib->data;
	size_t write_size = 0, diff2 = 0;
	int diff = 0;

	// verificare permisiuni
	if (check_permissions_write(node_minib) == 0) {
		printf("Invalid permissions for write.\n");
		return;
	}

	// incepere scriere
	node_minib = block->miniblock_list->head;
	while (write_size < size2) {
		miniblock = node_minib->data;
		if (address < miniblock->start_address + miniblock->size) {
			diff = (int)address - miniblock->start_address;
			if (diff < 0)
				diff = 0;
			diff2 = miniblock->size;
			if (diff2 > size2 - write_size)
				diff2 = size2 - write_size;
			if (!miniblock->rw_buffer) {
				miniblock->rw_buffer = malloc(miniblock->size + 1);
				DIE(!miniblock->rw_buffer, "Alocare esuata");
			}
			strncpy(((char *)miniblock->rw_buffer) + diff,
					((char *)data) + write_size, (int)diff2);

			write_size += diff2;
		}
		node_minib = node_minib->next;
	}
}

// se determina numarul de miniblockuri total
int det_nr_miniblocks(const arena_t *arena)
{
	dll_node_t *node = arena->alloc_list->head;
	int count = 0;
	while (node) {
		count += ((block_t *)node->data)->miniblock_list->size;
		node = node->next;
	}
	return count;
}

// se calculeaza memoria folosita
uint64_t memory_use(const arena_t *arena)
{
	uint64_t memory = 0;
	dll_node_t *node = arena->alloc_list->head;
	for (size_t i = 0; i < arena->alloc_list->size; i++) {
		memory = memory + ((block_t *)node->data)->size;
		node = node->next;
	}
	return memory;
}

// se printeza permisiunile unui miniblock
void print_permissions(miniblock_t *miniblock)
{
	if (miniblock->perm == 0) {
		printf("---\n");
		return;
	}

	if (miniblock->perm == 1) {
		printf("--X\n");
		return;
	}

	if (miniblock->perm == 2) {
		printf("-W-\n");
		return;
	}

	if (miniblock->perm == 4) {
		printf("R--\n");
		return;
	}

	if (miniblock->perm == 5) {
		printf("-WX\n");
		return;
	}
	if (miniblock->perm == 6) {
		printf("RW-\n");
		return;
	}
	if (miniblock->perm == 7) {
		printf("RWX\n");
		return;
	}
}

void pmap(const arena_t *arena)
{
	dll_node_t *node = arena->alloc_list->head;
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);

	//se calculeaza memoria libera
	uint64_t free_mem = arena->arena_size - memory_use(arena);
	printf("Free memory: 0x%lX bytes\n", free_mem);
	printf("Number of allocated blocks: %ld\n", arena->alloc_list->size);
	int nr_minib = det_nr_miniblocks(arena);
	printf("Number of allocated miniblocks: %d\n", nr_minib);

	// se afiseaza pentru fiecare block din lista
	for (size_t i = 0; i < arena->alloc_list->size; i++) {
		block_t *block = node->data;
		printf("\nBlock %ld begin\n", i + 1);
		uint64_t end_address = block->start_address + block->size;
		printf("Zone: 0x%lX - 0x%lX\n", block->start_address, end_address);

		// se afiseaza pentru fiecare miniblock dintr-un block
		dll_node_t *node_mb = block->miniblock_list->head;
		for (size_t j = 0; j < block->miniblock_list->size; j++) {
			miniblock_t *minib = node_mb->data;
			printf("Miniblock %ld:", j + 1);
			uint64_t end = minib->start_address + minib->size;
			printf("\t\t0x%lX\t\t-\t\t0x%lX\t\t| ", minib->start_address, end);
			print_permissions(minib);
			node_mb = node_mb->next;
		}
		printf("Block %ld end\n", i + 1);
		node = node->next;
	}
}

void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	dll_node_t *curr = arena->alloc_list->head;
	block_t *block;
	int ok = 0;
	// se verifica daca adresa se regaseste intr-un block
	while (curr) {
		block = curr->data;
		uint64_t start = block->start_address;
		if (address >= start && address < start + block->size) {
			ok = 1;
			break;
		}
		curr = curr->next;
	}
	if (ok == 0) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	// parcurgere pana la nodul care cuprinde adresa
	curr = arena->alloc_list->head;
	block = curr->data;
	while (address > block->start_address + block->size)
		curr = curr->next;

	block = curr->data;

	ok = 0;
	dll_node_t *node = block->miniblock_list->head;

	//verificare si aplicare permisiuni
	while (node) {
		if (address == ((miniblock_t *)node->data)->start_address) {
			((miniblock_t *)node->data)->perm = *permission;
			ok = 1;
		}
		node = node->next;
	}

	// daca nu s-a gasit niciun miniblock care sa inceapa cu adresa primita
	if (ok == 0) {
		printf("Invalid address for mprotect.\n");
		return;
	}
}
