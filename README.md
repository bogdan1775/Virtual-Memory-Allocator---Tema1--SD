# Croitoru Constantin-Bogdan
# Virtual-Memory-Allocator---Tema1--SD

Tema propune implementarea unui alocator de memorie care are rolul de a rezerva memorie, la nivel de biblioteca, traditional prin apeluri de memorie precum malloc() sau calloc(). Acestea marchează ca fiind folosite anumite zone de memorie dintr-un pool de bytes prealocat, numit arenă. Deasemenea, alocatorul de memorie se ocupă și cu eliberarea zonelor rezervate, apelul de bibliotecă aferent fiind free().


Comenzi posibile (40p)
Input-ul este oferit de la stdin, iar output-ul la stdout, respectând formatul următor:

ALLOC_ARENA <dimensiune_aren>
Se alocă un buffer contiguu de dimensiune ce va fi folosit pe post de kernel buffer sau arenă. Alocarea este pur virtuală, adică acest buffer este folosit doar pentru a simula existența unui spațiu fizic aferent zonelor de memorie înlănțuite.
DEALLOC_ARENA
Se eliberează arena alocată la începutul programului. Cum alocarea arenei a fost făcută virtual, această comandă rezultă în eliberarea tuturor resurselor folosite, precum lista de block-uri și listele de miniblock-uri asociate acestora.
ALLOC_BLOCK <adresă_din_arenă> <dimensiune_block>
Se marchează ca fiind rezervată o zonă ce începe la adresa <adresă_din_arenă> în kernel buffer cu dimensiunea de <dimensiune_block>
Dacă nicio adresă din zona de memorie [adresă_din_arenă, adresă_din_arenă + dimensiune) nu a mai fost alocată anterior și nu există alocate zonele de memorie [x, adresă_din_arenă - 1) și [adresă_din_arenă + dimensiune + 1, y], unde x < adresă_din_arenă - 1 și y > adresă_din_arenă + dimensiune + 1, atunci se inserează un nou block în lista de zone alocate.
Dacă nicio adresă din zona de memorie [adresă_din_arenă, adresă_din_arenă + dimensiune) nu a mai fost alocată anterior și există alocate zonele de memorie [x, adresă_din_arenă - 1) sau [adresă_din_arenă + dimensiune + 1, y], unde x < adresă_din_arenă - 1 și y > adresă_din_arenă + dimensiune + 1, atunci se șterge block-ul/block-urile adiacente din lista de zone alocate și se adaugă la lista internă de miniblock-uri a noii zone contigue de memorie. Cu alte cuvinte, zonele adiacente din memorie vor fi mereu în același block.
FREE_BLOCK <adresă_din_arenă>
Se eliberează un miniblock.
Dacă se eliberează unicul miniblock din cadrul unui block, atunci block-ul este la rândul său eliberat.
Dacă se eliberează un miniblock de la începutul/sfârșitul acestuia, atunci structura block-ului nu se modifică.
Dacă se eliberează un miniblock din mijlocul block-ului, atunci acesta va fi împărțit în două block-uri distincte.
READ <adresă_din_arenă> <dimensiune>
Se afișează <dimensiune> bytes începând cu adresa <adresă_din_arenă>, iar la final \n.
Dacă block-ul nu conține <dimensiune> bytes începând cu adresa <adresă_din_arenă>, se va afișa “Warning: size was bigger than the block size. Reading <size> characters.\n” și se vor afișa datele disponibile.
WRITE <adresă_din_arenă> <dimensiune> <date>
Se scriu <dimensiune> bytes din <date> la adresa <adresă_din_arenă>.
Dacă <date> nu conține <dimensiune> bytes pe același rând, se va citi în continuare, până la atingerea dimensiunii stabilite.
Dacă block-ul nu conține <dimensiune> bytes începând cu adresa <adresă_din_arenă>, se va afișa “Warning: size was bigger than the block size. Writing <size> characters.\n”.
PMAP
Tradițional, apelul pmap(), este folosit pentru a vizualiza zonele de memorie utilizate de un proces. Printre acestea se numără .text, .bss, .data, .rodata, etc, însă voi veți avea de implementat o funcționalitate mai facilă.
Se afișează informații despre block-urile și miniblock-urile existente.
Formatul permisiunilor este RWX. Dacă vreuna dintre aceste permisiuni lipsește, ea va fi înlocuită cu -.


Tratarea erorilor (40p)
La primirea comenzilor, pot apărea erori de input. Pe acestea, voi trebuie să le tratați corespunzător, prin afișarea unui mesaj sugestiv:

INVALID_ALLOC_BLOCK
Dacă adresa de început a blocului ce se dorește a fi alocat depășește dimensiunea arenei, se va afișa “The allocated address is outside the size of arena\n”.
Dacă adresa de final a blocului ce se dorește a fi alocat depășește dimensiunea arenei, se va afișa “The end address is past the size of the arena\n”.
Dacă există cel puțin o adresă din zona de memorie [adresă_din_arenă, adresă_din_arenă + dimensiune) care a fost alocată anterior, se va afișa “This zone was already allocated.\n”.
INVALID_ADDRESS_FREE
Dacă se încearcă eliberarea zonei de memorie asociate unei adrese invalide (nu a fost alocată sau nu reprezintă o adresă de început de miniblock),se afișează “Invalid address for free.\n”.
INVALID_ADDRESS_READ
Dacă se încearcă citirea de la o adresă invalidă (nu a fost alocată), se afișează “Invalid address for read.\n”.
INVALID_ADDRESS_WRITE
Dacă se încearcă scrierea la o adresă invalidă (nu a fost alocată), se afișează “Invalid address for write.\n”.
INVALID_COMMAND
Dacă este introdusă o comandă inexistentă sau dacă numărul de parametri al acesteia este incorect, se afișează “Invalid command. Please try again.\n” și se va trece la următoarea comandă.
Bonus (20p)
MPROTECT <adresă_din_arenă> <noua_permisiune>
Schimbă permisiunile zonei care începe la <adresă_din_arenă> de dimensiune <dimensiune> din default-ul RW- în <noua_permisiune>.
Noua permisiune este dată ca mască de biți (reținute ca string-uri), astfel:
PROT_NONE: Memoria nu poate fi accesată deloc; în octal, masca este reprezentată prin 0
PROT_READ: Memoria poate fi citită; în octal, masca este reprezentată prin 4
PROT_WRITE: Memoria poate fi scrisă; în octal, masca este reprezentată prin 2
PROT_EXEC: Memoria poate fi executată; în octal, masca este reprezentată prin 1
Astfel, o zonă cu permisiunile RW-, are reținut, în octal, flag-ul 6 , care este și valoarea default a unei zone nou alocate.
Va trebui să citiți string-urile primite de la tastatură și să le interpretați numeric. Pentru permisiuni multiple, acestea vor fi înlănțuite prin operatorul |.
Prin schimbarea permisiunilor, în cadrul operațiile de READ și WRITE trebuie să se verifice dacă există permisiuni pentru aplicarea acestora. Dacă acestea nu există se va afișa, după caz, “Invalid permissions for read.\n” sau “Invalid permissions for write.\n”.
Dacă se încearcă schimbarea permisiunilor zonei de memorie asociate unei adrese invalide (nu a fost alocată sau nu reprezintă o adresă de început de miniblock), se afișează “Invalid address for mprotect.\n”.
