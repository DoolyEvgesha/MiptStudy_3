/*
 * There are 4 functions: one putting a screw on the car,
 * two other are putting bolts on the car,
 *  the last one check whether the car is fully built.
 *  Useful functions:pthread_mutex_lock(unlock), pthread_mutex_init, pthread_create, pthread_join,
 *  pthread_mutex_destroy;
 *  PS: more entertaining to work with pizza: two pieces of ham and one piece of cheese))))
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <assert.h>

pthread_t ntid[4];

struct hoare {
    struct Pizza {
        char pizza[4];
        long letter;
        int cheese;
        int ham;
    } p;

    struct Queue {
        char **pizzas;
        int size;
        int used;
    } q;

    long numPizzas;
    long countMade;
    long countChecked;

    pthread_mutex_t lock;
    pthread_cond_t pizzaIsReady;

    int (*putElement)(struct hoare *monitor, char letter);
    int (*checkPizza)(struct hoare *monitor);
};

int pizzaReady(struct hoare *mon) {
    if((mon->p.cheese == 1) && (mon->p.ham == 2)) {
        mon->countMade++;
        return 1;
    }
    else return 0;
}

void newPizza(struct hoare *mon) {
    memset(&(mon->p), 0, sizeof(mon->p));
}

int putElement(struct hoare *mon, char letter) {
    pthread_mutex_lock(&mon->lock);
    int c = 0;
    int noSpace = 0;
    int cheeser = 0;
    if(letter == 'C') cheeser = 1;
    if(cheeser) noSpace = mon->p.cheese == 1;
    else noSpace = mon->p.ham == 2;

    if (mon->countMade < mon->numPizzas) {
        while (noSpace) {
            pthread_cond_wait(&mon->pizzaIsReady, &mon->lock);
            if(cheeser) noSpace = mon->p.cheese == 1;
            else noSpace = mon->p.ham == 2;
            if(mon->countMade >= mon->numPizzas) goto end;
        }

        mon->p.pizza[mon->p.letter] = letter;

        printf("pizza[%ld] = %c,countMade = %ld, numPizzas = %ld\n",mon->p.letter, mon->p.pizza[mon->p.letter],
               mon->countMade, mon->numPizzas);

        assert(mon->p.letter < 3);
        mon->p.letter++;
        if(cheeser) mon->p.cheese++;
        else mon->p.ham++;

        if (pizzaReady(mon)) {
            printf("READY\n");
            if (mon->q.used >= mon->q.size) {
                mon->q.size += 10;
                mon->q.pizzas = realloc(mon->q.pizzas, mon->q.size * sizeof(char*));
            }
            mon->q.pizzas[mon->q.used] = strdup(mon->p.pizza);
            printf("**cur pizza = %s****\n", mon->q.pizzas[mon->q.used]);
            mon->q.used++;

            newPizza(mon);
            pthread_cond_broadcast(&(mon->pizzaIsReady));
        }
    }
    end:
    c = mon->countMade == mon->numPizzas;

    pthread_mutex_unlock(&mon->lock);
    return c;
}

int checkPizza(struct hoare *mon) {
    pthread_mutex_lock(&mon->lock);

    int ret = 0;
    while(mon->countMade == mon->countChecked)
        pthread_cond_wait(&(mon->pizzaIsReady), &(mon->lock));

    if ((mon->q.pizzas[mon->countChecked][0] +
         mon->q.pizzas[mon->countChecked][1] +
         mon->q.pizzas[mon->countChecked][2])
        != ('C' + 'H' + 'H'))
        ret++;
    //if ret == 0 then pizza is good

    mon->countChecked++;

    pthread_mutex_unlock(&mon->lock);
    return ret;
}

void *hammer(void *p) {
    int numWork = 0;
    struct hoare *monitor = p;
    while (!numWork) {
        numWork = monitor->putElement(monitor, 'H');
    }
    return NULL;
}

void *cheeser(void *p) {
    int numWork = 0;
    struct hoare *monitor = p;
    while (!numWork) {
        numWork = monitor->putElement(monitor, 'C');
    }
    return NULL;
}

void *checker(void *p) {
    struct hoare *monitor = p;

    int ans = 0;

    for (int i = 0; i < monitor->numPizzas; i++)
        ans += monitor->checkPizza(monitor);

    //if ans == 0 then pizza is good
    if (ans == 0)
        printf("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n");
    else
        printf("Damn it!\n");
    return NULL;
}

void createHoare(struct hoare *mon, long numPizzas) {
    memset(mon, 0, sizeof(*mon));

    if (pthread_mutex_init(&mon->lock, NULL) != 0) {
        fprintf(stderr, "\n mutex init failed\n");
        perror("Lock\n");
    }
    pthread_cond_init(&(mon->pizzaIsReady), NULL);

    mon->putElement = putElement;
    mon->checkPizza = checkPizza;

    mon->q.size = 10;
    mon->q.pizzas = (char**)calloc(mon->q.size, sizeof(char*));

    mon->numPizzas = numPizzas;
}

void freeMonitor(struct hoare * mon){
    for(int i = 0; (i < mon->q.size) && (mon->q.pizzas[i]);i++)
        free(mon->q.pizzas[i]);
    free(mon->q.pizzas);
}


int main(int argc, char *argv[]) {
    int err;
    long numPizzas = strtol(argv[1], NULL, 10);

    struct hoare monitor;
    createHoare(&monitor, numPizzas);

    err = pthread_create(&ntid[0], NULL, &cheeser, &monitor);
    if (err != 0) {
        perror("Error with putCheese");
        return 1;
    }

    err = pthread_create(&ntid[1], NULL, &hammer, &monitor);
    if (err != 0) {
        perror("Error with putHam");
        return 1;
    }

    err = pthread_create(&ntid[2], NULL, &hammer, &monitor);
    if (err != 0) {
        perror("Error with putHam");
        return 1;
    }

    err = pthread_create(&ntid[3], NULL, &checker, &monitor);
    if (err != 0) {
        perror("Error with checkPizza");
        return 1;
    }

    for (int i = 0; i < 4; i++)
        pthread_join(ntid[i], NULL);

    pthread_mutex_destroy(&monitor.lock);
    freeMonitor(&monitor);
    return 0;
}
