#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

const int taille = 5000;

const int xmin = -1;
const int xmax = 1;
const int ymin = -1;
const int ymax = 1;
const int iterationmax = 100;

const double a = -0.8;
const double b = 0.156;

typedef struct {
    int R;
    int G;
    int B;
} pixel;

typedef struct {
    pixel **pixels;
    int size;
    int line_if_thread_by_col;
    int start;
} thread_struct;

void write_image_ppm(pixel **pixels) {
    if (!pixels) {
        return;
    }
    FILE *fptr;
    
    fptr = fopen("rendu.ppm", "w");

    if (fptr == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    fprintf(fptr, "P3\n");
    fprintf(fptr, "%d %d\n", taille, taille);
    fprintf(fptr, "255\n");

    for (int line = 0; line < taille; ++line) {
        for (int col = 0; col < taille; ++col) {
            fprintf(fptr, "%d %d %d\n",
                pixels[col][line].R,
                pixels[col][line].G,
                pixels[col][line].B);
        }
    }

    fclose(fptr); 
}

void col_to_line_matrix(pixel **pixels) {
    if (!pixels) {
        return;
    }
    pixel tmp;

    for (int line = 0; line < taille; ++line) {
        for (int col = line + 1; col < taille; ++col) {
            tmp = pixels[line][col];
            pixels[line][col] = pixels[col][line];
            pixels[col][line] = tmp;
        }
    }
}

pixel **julia_ite() {
    pixel **pixels = malloc(sizeof(pixel *) * taille);
    for (int i = 0 ; i < taille ; ++i) {
        pixels[i] = malloc(sizeof(pixel) * taille);
    }
    for (int line = 0 ; line < taille ; ++line) {
        for (int col = 0 ; col < taille ; ++col) {
            int i = 1;
            double x = xmin + (double)col*(xmax-xmin)/taille;
            double y = ymax - (double)line*(ymax-ymin)/taille;

            while ((i<=iterationmax) && ((x*x + y*y)<=4)) {
                double temp = x*x - y*y + a;
                y = 2*x*y + b;
                x = temp;

                ++i;   
            }
            if ((i>iterationmax) && ((x*x + y*y)<=4)) {
                pixels[line][col].R = 0;
                pixels[line][col].G = 0;
                pixels[line][col].B = 0;
            } else {
                pixels[line][col].R = (4*i)%256;
                pixels[line][col].G = (2*i)%256;
                pixels[line][col].B = (6*i)%256;
            }
        }
    }
    return pixels;
}

void *thread_work_line(void *th_struct) {
    thread_struct *th_st = (thread_struct *) th_struct;
    for (int line = 0 ; line < th_st->size ; ++line) {
        for (int col = 0 ; col < taille ; ++col) {
            int i = 1;
            double x = xmin + (double)col*(xmax-xmin)/taille;
            double y = ymax - (double)(line + th_st->start)*(ymax-ymin)/taille;

            while ((i<=iterationmax) && ((x*x + y*y)<=4)) {
                double temp = x*x - y*y + a;
                y = 2*x*y + b;
                x = temp;

                ++i;   
            }
            if ((i>iterationmax) && ((x*x + y*y)<=4)) {
                th_st->pixels[line][col].R = 0;
                th_st->pixels[line][col].G = 0;
                th_st->pixels[line][col].B = 0;
            } else {
                th_st->pixels[line][col].R = (4*i)%256;
                th_st->pixels[line][col].G = (2*i)%256;
                th_st->pixels[line][col].B = (6*i)%256;
            }   
        }
    }
    //return (void *) &(th_st->pixels);
    pthread_exit(EXIT_SUCCESS);
}

/*
    Pour etre efficace (cache miss), inverser la matrice (col becomes lines) puis reinverser
*/
// void *thread_work_col(void *th_struct) {
//     thread_struct *th_st = (thread_struct *) th_struct;

//     for (int col = 0 ; col < th_st->size ; ++col) {
//         int i = 1;
//         double x = xmin + (double)col*(xmax-xmin)/taille;
//         double y = ymax - (double)th_st->line_if_thread_by_col*(ymax-ymin)/taille;

//         while ((i<=iterationmax) && ((x*x + y*y)<=4)) {
//             double temp = x*x - y*y + a;
//             y = 2*x*y + b;
//             x = temp;

//             ++i;   
//         }
//         if ((i>iterationmax) && ((x*x + y*y)<=4)) {
//             th_st->pixels[th_st->line_if_thread_by_col][col].R = 0;
//             th_st->pixels[th_st->line_if_thread_by_col][col].G = 0;
//             th_st->pixels[th_st->line_if_thread_by_col][col].B = 0;
//         } else {
//             th_st->pixels[th_st->line_if_thread_by_col][col].R = (4*i)%256;
//             th_st->pixels[th_st->line_if_thread_by_col][col].G = (2*i)%256;
//             th_st->pixels[th_st->line_if_thread_by_col][col].B = (6*i)%256;
//         }
//     }
//     return (void *) &(th_st->pixels);
// }

/*
    Bon, diviser les colonnes pour les threads 
*/
void *thread_work_col(void *th_struct) {
    thread_struct *th_st = (thread_struct *) th_struct;

    for (int line = 0 ; line < taille ; ++line) {
        for (int col = th_st->start ; col < th_st->start + th_st->size ; ++col) {
            int i = 1;
            double x = xmin + (double)col*(xmax-xmin)/taille;
            double y = ymax - (double)line*(ymax-ymin)/taille;

            while ((i<=iterationmax) && ((x*x + y*y)<=4)) {
                double temp = x*x - y*y + a;
                y = 2*x*y + b;
                x = temp;

                ++i;   
            }
            if ((i>iterationmax) && ((x*x + y*y)<=4)) {
                th_st->pixels[line][col].R = 0;
                th_st->pixels[line][col].G = 0;
                th_st->pixels[line][col].B = 0;
            } else {
                th_st->pixels[line][col].R = (4*i)%256;
                th_st->pixels[line][col].G = (2*i)%256;
                th_st->pixels[line][col].B = (6*i)%256;
            }
        }
    }

    //return (void *) &(th_st->pixels);
    pthread_exit(EXIT_SUCCESS);
}

pixel **julia_threaded(int nb_thread, void *(*fnct)(void *), int line_or_col) {
    int taille_par_thread = taille / nb_thread;

    pixel **pixels = malloc(sizeof(pixel *) * taille);
    for (int i = 0 ; i < taille ; ++i) {
        pixels[i] = malloc(sizeof(pixel) * taille);
    }

    pthread_t threads[nb_thread];
    thread_struct th_st[nb_thread];
    int start = 0;

    for (int i = 0 ; i < nb_thread ; ++i) {
        start = i * taille_par_thread;
        //th_st[i].pixels = line_or_col == 1 ? &(pixels[start]) : pixels;
        if (line_or_col == 1) { // by line
            th_st[i].pixels = &(pixels[i * taille_par_thread]);
        }
        if (line_or_col == 2) { // by col
            th_st[i].pixels = pixels;
        }
        th_st[i].size = taille_par_thread;
        th_st[i].start = start;
        
        pthread_create(&(threads[i]), NULL, fnct, (void *) &(th_st[i]));
        //fnct((void *) &th_st[i]);
    }
    for (int i = 0 ; i < nb_thread ; ++i) {
        pthread_join(threads[i], NULL);
    }
    return pixels;
}

// pixel **julia_threaded(int nb_thread, void *(*fnct)(void *)) {
//     int taille_par_thread = taille / nb_thread;

//     pixel **pixels = malloc(sizeof(pixel *) * taille);
//     for (int i = 0 ; i < taille ; ++i) {
//         pixels[i] = malloc(sizeof(pixel) * taille);
//     }

//     thread_struct th_st[nb_thread];

//     for (int line = 0 ; line < taille ; ++line) {
//         for (int i = 0 ; i < nb_thread ; ++i) {
//             th_st[i].line_if_thread_by_col = line;
//             th_st[i].pixels = &(pixels[i * taille_par_thread]);
//             th_st[i].size = taille_par_thread;
            
//             fnct((void *) &th_st[i]);
//         }
//     }
//     return pixels;
// }

int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("usage : ./julia nb_thread method (being iterative (0), threaded by line (1), threaded by col (2))");
        exit(1);
    }
    int nb_thread = atoi(argv[1]);
    if (!(0 < nb_thread && nb_thread <= 16)) {
        perror("Votre nombre de threads doit etre supérieur à 0 et inférieur à 16");
        exit(1);
    }

    pixel **pixels = NULL;
    char texte[250];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    sprintf((texte), "%d-%02d-%02d %02d:%02d:%02d - ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(&(texte[strlen(texte)]), "%d * %d pixels - ", taille, taille);
    sprintf(&(texte[strlen(texte)]), "Méthode utilisée : ");
    time_t time_start, time_end;
    time_start = time(NULL);

    switch (atoi(argv[2]))
    {
    case 0:
        sprintf(&(texte[strlen(texte)]), "itératif ");
        pixels = julia_ite();
        break;
    case 1:
        sprintf(&(texte[strlen(texte)]), "%d thread sur portion de lignes entières ", nb_thread);
        pixels = julia_threaded(nb_thread, thread_work_line, 1);
        break;
    case 2:
        sprintf(&(texte[strlen(texte)]), "%d thread sur portion de colonnes entières ", nb_thread);
        pixels = julia_threaded(nb_thread, thread_work_col, 2);
        break;
    default:
        break;
    }
    if (pixels) {
        write_image_ppm(pixels);
        time_end = time(NULL);
        double tpsExec = difftime(time_end, time_start);
        printf("Temps exécution : %fs\n", tpsExec);
        sprintf(&(texte[strlen(texte)]), "Temps exécution : %fs", tpsExec);
        FILE *fptr;
    
        fptr = fopen("exec_time.txt", "aw");

        if (fptr == NULL) {
            perror("Erreur lors de l'ouverture du fichier");
            return 1;
        }

        fprintf(fptr, "%s\n", texte);

        for (int i = 0 ; i < taille ; ++i) {
            free(pixels[i]);
        }
        free(pixels);
    }    

    return 0;
}
