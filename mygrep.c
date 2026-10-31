#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Constantes globales
#define EPS 256
#define ALL 257
#define MATCH 258

#define MAX_LINE_LENGTH 1024

// Structure pour les états d'un automate
struct state {
    int c;
    struct state* out1;
    struct state* out2;
};
typedef struct state state;

// Structure pour un automate non déterministe à epsilon-transitions
struct nfa {
    state* start;
    state* final;
    int n;
};
typedef struct nfa nfa;

// Structure pour manipuler des piles
// length contient la taille courante de la pile, capacity contient la taille maximale de la pile et data et un tableau de taille capacity dont les seuls éléments manipulables sont avant l'indice length strictement.
struct stack {
    int length;
    int capacity;
    nfa** data;
};
typedef struct stack stack;

// Création d'une pile
stack* stack_new(int capacity)
{
    stack* s = malloc(sizeof(stack));
    s->data = malloc(capacity * sizeof(nfa*));
    s->capacity = capacity;
    s->length = 0;
    return s;
}

// Extraction d'une pile
nfa* pop(stack* s){
    assert(s->length > 0);
    nfa* result = s->data[s->length - 1];
    s->length--;
    return result;
}

// Ajout dans une pile
void push(stack* s, nfa* a){
    assert(s->capacity > s->length);
    s->data[s->length] = a;
    s->length++;
}

/*QUESTION 3*/
state* new_state(int c, state* out1, state* out2) {
    state* s = malloc(sizeof(state));
    s->c = c;
    s->out1 = out1;
    s->out2 = out2;
    return s;
}
/*QUESTION 4*/
nfa* character(int c) {
    // Création des deux états : start et final
    state* s1 = new_state(c, NULL, NULL);
    state* s2 = new_state(MATCH, NULL, NULL);

    // L’état de départ pointe vers le final via c
    s1->out1 = s2;

    nfa* a = malloc(sizeof(nfa));

    a->start = s1;
    a->final = s2;
    a->n = 2;

    return a;
}

/*QUESTION 5*/
nfa* all(void) {
    state* s1 = new_state(ALL, NULL, NULL);
    state* s2 = new_state(MATCH, NULL, NULL);
    s1->out1 = s2;

    nfa* a = malloc(sizeof(nfa));

    a->start = s1;
    a->final = s2;
    a->n = 2;

    return a;
}
/*QUESTION 6*/
nfa* concat(nfa* a, nfa* b) {
    //l’état final de a devient une epsilon transition vers les états initiaux de b 
    a->final->c = EPS;
    a->final->out1 = b->start;
    a->final->out2 = NULL;

    // On doit faire un nouveau automate
    nfa* res = malloc(sizeof(nfa));
    res->start = a->start;
    res->final = b->final;
    res->n = a->n + b->n;
    return res;
}
nfa* alternative(nfa* a, nfa* b) { // C'est pour |
    // On doit creer doit nouveaux états initiaux et finaux
    // POur correspondre à la construction du cours
    state* s_start = new_state(EPS, a->start, b->start);
    state* s_final = new_state(MATCH, NULL, NULL);
    
    a->final->c = EPS;
    a->final->out1 = s_final;
    a->final->out2 = NULL;
    
    b->final->c = EPS;
    b->final->out1 = s_final;
    b->final->out2 = NULL;
    
    // On doit faire un nouveau automate
    nfa* res = malloc(sizeof(nfa));
    res->start = s_start;
    res->final = s_final;
    res->n = a->n + b->n + 2; //on ajoute deux états pour que l’autotomate soit généralisé
    return res;
}

nfa* star(nfa* a) {

    state* s_final = new_state(MATCH, NULL, NULL);
    
    state* s_start = new_state(EPS, a->start, s_final); 
    //ici on fait une deux transitions avec epsilons :
    //une pour arriver à l'automate
    //une deuxième pour lire le mot epsilon

    a->final->c = EPS;
    a->final->out1 = a->start;
    a->final->out2 = s_final;
    //ici une sortie c’est pour faire la boucle de l'etoile et l'autre pour finir
    
    nfa* res = malloc(sizeof(nfa));

    res->start = s_start;
    res->final = s_final;
    res->n = a->n + 2; //idem que pour alternative
    return res;
}

    nfa* maybe(nfa* a) {
        state* s_final = new_state(MATCH, NULL, NULL);
        state* s_start = new_state(EPS, a->start, s_final);
        //soit on lit le mot, soit on va à la fin directement

        a->final->c = EPS;
        a->final->out1 = s_final;
        a->final->out2 = NULL;
        //c'est la même idée que pour étoile sauf que ici on ne fait pas de boucle

        s_start->out2 = s_final;

        nfa* res = malloc(sizeof(nfa));
        res->start = s_start;
        res->final = s_final;
        res->n = a->n + 2;
        return res;
    }

/*QUESTION 9*/
nfa* build(char* regex) {
    //On fait comme si on le faisait à la main en lisant de gauche à droite caractère par caractère
    int len = strlen(regex);
    stack* s = stack_new(len);

    for (int i = 0; i < len; i++) {
        char c = regex[i];
        nfa *a, *b;

        if (c == '@') { // concaténation
            //Normalement quand on fait la concatenation il y a deux elements dans la pile
            //Donc aucun  pour extraire des éléments avec une expression qui est valide
            b = pop(s);
            a = pop(s);
            push(s, concat(a, b));

        } else if (c == '|') { // union
            b = pop(s);
            a = pop(s);
            push(s, alternative(a, b));

        } else if (c == '*') { // étoile
            a = pop(s);
            push(s, star(a));

        } else if (c == '?') { // peut-être
            a = pop(s);
            push(s, maybe(a));

        } else if (c == '.') { // tout caractère
            push(s, all());

        } else { // caractère ordinaire
            push(s, character(c));
        }
        printf("%d\n", s->length);
    }

    // À la fin, il doit rester un seul automate
    assert(s->length == 1);
    nfa* result = pop(s);
    free(s->data);//on libère la pile qui ne sera plus utile
    free(s);
    return result;
}

//Fonction mystère pour la question 11
bool mystery(state* state, char* s)
{
    if (state == NULL) return false;
    if (state->c == EPS) {
        return mystery(state->out1, s) || mystery(state->out2, s);
    }
    if (s[0] == '\0' || s[0] == '\n') return  state->c == MATCH;
    if (s[0] == state->c || state->c == ALL) return mystery(state->out1, &s[1]);
    return false;
}
//Reconnaissance de mot par un automate 
bool accept(nfa a, char* s)
{
    return mystery(a.start, s);
}
//Fonction grep
void match_stream(nfa a, FILE* in)
{
    char* line = malloc((MAX_LINE_LENGTH + 1) * sizeof(char));
    while (true) 
    {
        if (fgets(line, MAX_LINE_LENGTH, in) == NULL) 
        {
            break;
        }
        if (accept(a, line)) 
        {
            printf("%s", line);
        }
    }
    free(line);
}


void free_accessible_states(state* q)
{
if (q == NULL) return;
free_accessible_states(q->out1);
free_accessible_states(q->out2);
free(q);
}
void free_automaton(nfa* a)
{
free_accessible_states(a->start);
free(a);
}

int main(int argc, char* argv[])
{
/*QUESTION 12*/

    if (argc != 3) {
        printf("%s : il faut mettre les arguments de la manière <expression> <fichier>\n", argv[0]);
        return 0;
    }

    char* regex = argv[1];
    char* filename = argv[2];

    nfa* automate = build(regex); 
    FILE* file = fopen(filename, "r");
    if (file == NULL){
        printf("le fichier \"%s\" n'est pas valide\n", filename);
        return 0;
    }
    printf("=== Lignes acceptées par l'expression : \"%s\" ===\n", regex);
    match_stream(*automate, file);

    fclose(file);
    free_automaton(automate);
    return 0;
}



/*

//COMPILER AVEC MÉMOIRE//

    gcc -g -Wall -Wextra -fsanitize=address -o mygrep.exe mygrep.c

//COMPILER SANS MÉMOIRE//
    
    gcc -g -Wall -Wextra -o mygrep.exe mygrep.c

*/