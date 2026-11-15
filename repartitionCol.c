#include <stdio.h>
#include <stdlib.h>

/*
    Simplement voir si toutes les colonnes sont traitées.
*/
int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("Usage : ./repartitionCol nb_col nb_threads\n");
        exit(1);
    }
    int nb_col = atoi(argv[1]);
    int nb_th = atoi(argv[2]);

    int nb_col_by_th = nb_col / nb_th;

    int st_col[nb_th];
    int end_col[nb_th];

    for (int i = 0 ; i < nb_th ; ++i) {
        st_col[i] = i * nb_col_by_th;
        end_col[i] = st_col[i] + nb_col_by_th;
    }
    end_col[nb_th - 1] = nb_col;

    printf("Repartition des colonnes : \n");
    for (int i = 0 ; i < nb_th ; ++i) {
        printf("Thread n°%d : %d -> %d\n", i, st_col[i], end_col[i]);
    }

    return 0;
}