/* Data Structure for Statistics
 * A simple data structure for statistics in C
 * Autor: Mateus da Silva
 * Data: 2024-05-08
 * Contato: ext.mateus@gmail.com
 *
 * TODO:
 * validar os dados numericos
 * quando o input estiver errado mostrar como deve ser
 * datetime
 * boolean
 * retornar uma serie
 * criar e retornar um dataframe
 * criar e retornar um recorte do dataframe
 * groupby
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum types { INT, FLOAT, STRING };

struct frame {
  int rows;
  int cols;
  void ***data;
  char **header;
  enum types *types;
} frame;

struct container {
  struct frame *f[16];
  int size;
  char **c_name;
} container;

int container_index(struct container *c, char *c_name) {
  for (int i = 0; i < c->size; i++) {
    if (strcmp(c->c_name[i], c_name) == 0) {
      return i;
    }
  }
  return -1;
}

bool is_integer(char *s) {
  if (strlen(s) == 0) {
    return false;
  }

  while (*s != '\0') {
    if (!isdigit(*s)) {
      return false;
    }
    s++;
  }
  return true;
}

bool is_float(char *s) {
  if (strlen(s) == 0) {
    return false;
  }

  while (*s != '\0') {
    if (!isdigit(*s) && *s != '.') {
      return false;
    }
    s++;
  }
  return true;
}

int _infer_header(char **header_list, FILE *fp, char *sep) {
  rewind(fp);
  char *line = NULL;
  size_t len = 0;

  getline(&line, &len, fp);

  char *token = NULL;
  int cols = 0;

  while ((token = strsep(&line, sep))) {
    char *p = strchr(token, '\n');
    if (p) {
      *p = '\0';
    }

    header_list[cols] = token;
    cols++;
  }
  return cols;
}

int _infer_types(enum types *type_list, FILE *fp, char *sep, bool header) {
  rewind(fp);
  char *line = NULL;
  size_t len = 0;

  char *type = NULL;
  int col = 0;

  if (header) {
    getline(&line, &len, fp);
  }
  getline(&line, &len, fp);
  while ((type = strsep(&line, sep))) {
    char *p = strchr(type, '\n');
    if (p) {
      *p = '\0';
    }

    type_list[col] = STRING;

    if (is_float(type))
      type_list[col] = FLOAT;

    if (is_integer(type))
      type_list[col] = INT;

    col++;
  }
  return col;
}

int _count_lines(FILE *fp, bool header) {
  rewind(fp);
  int n_lines = 0;
  char *line = NULL;
  size_t len = 0;

  if (header) {
    getline(&line, &len, fp);
  }

  while (getline(&line, &len, fp) != -1) {
    n_lines++;
  }

  return n_lines;
}

int _read_data(char ***data, FILE *fp, char *sep, bool header) {
  rewind(fp);
  char *line = NULL;
  size_t len = 0;
  int rows = 0;
  int cols = 0;
  if (header) {
    getline(&line, &len, fp);
  }
  while (getline(&line, &len, fp) != -1) {
    char *saveptr;
    cols = 0;
    char *token = strtok_r(line, sep, &saveptr);
    while (token != NULL) {
      char *p = strchr(token, '\n');
      if (p) {
        *p = '\0';
      }

      char *a_token = malloc(strlen(token) * sizeof(char));
      if (a_token == NULL) {
        printf("Erro ao alocar memória para a_token\n");
        return -1;
      }
      strcpy(a_token, token);
      data[rows][cols] = a_token;
      token = strtok_r(NULL, sep, &saveptr);
      cols++;
    }
    rows++;
  }
  return rows;
}

struct frame *new_frame_from_csv(char *filename, bool header, char *sep) {
  FILE *fp = fopen(filename, "r");
  char *line = NULL;
  size_t len = 0;
  long header_offset = 0;
  char **header_list = malloc(64 * sizeof(char *));
  enum types *type_list = malloc(64 * sizeof(enum types));

  if (fp == NULL) {
    printf("Erro ao abrir o arquivo\n");
    return NULL;
  }
  if (header_list == NULL) {
    printf("Erro ao alocar memória para header_list\n");
    return NULL;
  }
  if (type_list == NULL) {
    printf("Erro ao alocar memória para type_list\n");
    return NULL;
  }

  int n_cols = 0;
  if (header) {
    header_offset = ftell(fp);
    n_cols = _infer_header(header_list, fp, sep);
  }

  int a = _infer_types(type_list, fp, sep, header);
  assert(n_cols == a);
  int n_lines = _count_lines(fp, header);

  // Alocando espaço para os ponteiros dos dados
  // data é uma matriz de strings
  char ***data = malloc(n_lines * sizeof(char **));
  if (data == NULL) {
    printf("Erro ao alocar memória para data\n");
    return NULL;
  }
  for (int i = 0; i < n_lines; i++) {
    data[i] = malloc(n_cols * sizeof(char *));
    if (data[i] == NULL) {
      printf("Erro ao alocar memória para data[%d]\n", i);
      return NULL;
    }
  }

  // Leirura dos dados
  int b = _read_data(data, fp, sep, header);
  assert(n_lines == b);

  struct frame *f = malloc(sizeof(struct frame));
  if (f == NULL) {
    printf("Erro ao alocar memória para frame\n");
    return NULL;
  }
  f->rows = n_lines;
  f->cols = n_cols;
  f->header = header_list;
  f->data = (void *)data;
  f->types = type_list;

  fclose(fp);
  return f;
}

char *get_object(const struct frame *f, const int row, const int col) {
  return f->data[row][col];
}

char *get_header_list(struct frame *f) {
  char *out = malloc(64 * f->cols * sizeof(char));
  if (out == NULL) {
    printf("Erro ao alocar memória para out\n");
    return NULL;
  }
  memset(out, 0, 64 * f->cols * sizeof(char));

  for (int i = 0; i < f->cols; i++) {
    strcat(out, f->header[i]);
    if (i < f->cols - 1)
      strcat(out, ", ");
  }

  return out;
}

int head(char **out, const struct frame *f, const int n) {

  for (int i = 0; i < f->cols; i++) {
    strcat(*out, f->header[i]);
    if (i < f->cols - 1)
      strcat(*out, ", ");
  }

  strcat(*out, "\n");

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < f->cols; j++) {
      strcat(*out, get_object(f, i, j));
      if (j < f->cols - 1)
        strcat(*out, ", ");
    }
    strcat(*out, "\n");
  }

  return 0;
}

int _int_sum(struct frame *f, int col) {
  int sum = 0;
  for (int i = 0; i < f->rows; i++) {
    sum += atoi(get_object(f, i, col));
  }
  return sum;
}

float _float_sum(struct frame *f, int col) {
  float sum = 0;
  for (int i = 0; i < f->rows; i++) {
    char *obj = get_object(f, i, col);
    if (is_float(obj)) {
      sum += atof(obj);
    } else {
      printf("Erro: %s não é um número\n", obj);
    }
  }
  return sum;
}

char *int_to_string(int n) {
  char *out = malloc(64 * sizeof(char));
  if (out == NULL) {
    printf("Erro ao alocar memória para out\n");
    return NULL;
  }
  sprintf(out, "%d", n);
  return out;
}

char *float_to_string(float n) {
  char *out = malloc(64 * sizeof(char));
  if (out == NULL) {
    printf("Erro ao alocar memória para out\n");
    return NULL;
  }
  sprintf(out, "%f", n);
  return out;
}

void sum(struct frame *f, int col) {
  if (f->types[col] == INT) {
    printf("Sum of %s: %d\n", f->header[col], _int_sum(f, col));
  } else if (f->types[col] == FLOAT) {
    printf("Sum of %s: %f\n", f->header[col], _float_sum(f, col));
  } else {
  }
}

//float _mean(struct frame *f, int col) {
//  return atof(sum(f, col)) / (float)f->cols;
//}

//char *mean(struct frame *f, int col) {
//  if (f->types[col] == INT) {
//    float mean = _mean(f, col);
//    char *out = malloc(64 * sizeof(char));
//    if (out == NULL) {
//      printf("Erro ao alocar memória para out\n");
//      return NULL;
//    }
//    sprintf(out, "%f", mean);
//    return out;
//  } else if (f->types[col] == FLOAT) {
//    float mean = _mean(f, col);
//    char *out = malloc(64 * sizeof(char));
//    if (out == NULL) {
//      printf("Erro ao alocar memória para out\n");
//      return NULL;
//    }
//    sprintf(out, "%f", mean);
//    return out;
//  } else {
//    return "Not a number";
//  }
//}

int load(struct frame **f, char *option, char *filename) {
  char *p = strchr(filename, '\n');
  if (p) {
    *p = '\0';
  }

  if (strcmp(option, "csv") == 0) {
    *f = new_frame_from_csv(filename, true, ",");
    if (*f == NULL) {
      printf("Erro ao carregar o arquivo\n");
      return 1;
    }
  }
  return 0;
}

void unload(struct frame **f) {
  for (int i = 0; i < (*f)->rows; i++) {
    for (int j = 0; j < (*f)->cols; j++) {
      free((*f)->data[i][j]);
    }
    free((*f)->data[i]);
  }
  free((*f)->data);
  free((*f)->header);
  free((*f)->types);
  free(*f);
}

char *create_framename(const char *filename) {
  char *framename = malloc(strlen(filename) * sizeof(char));
  if (framename == NULL) {
    printf("Erro ao alocar memória para framename\n");
    return NULL;
  }
  strcpy(framename, filename);
  char *p = strchr(framename, '.');
  if (p) {
    *p = '\0';
  }
  return framename;
}

void handle_input(char **input, struct container **c) {
  char *cp = *input;
  char *saveptr;
  char *token = strtok_r(cp, " ", &saveptr);

  // EXIT
  if (strcmp(token, "exit\n") == 0) {
    exit(0);
  }

  // LOAD
  if (strcmp(token, "load") == 0) {
    char *option = strtok_r(NULL, " ", &saveptr);
    char *filename = strtok_r(NULL, " ", &saveptr);

    (*c)->f[(*c)->size] = malloc(sizeof(struct frame));
    struct frame *f = (*c)->f[(*c)->size];
    char *framename = create_framename(filename);
    (*c)->c_name[(*c)->size] = framename;

    if (load(&(*c)->f[(*c)->size], option, filename) == 0) {
      struct frame *f = (*c)->f[(*c)->size];
      printf("Arquivo %s carregado como: %s\n %d linhas e %d colunas\n",
             filename, framename, f->rows, f->cols);
      (*c)->size++;
    }
  }

  // UNLOAD
  if (strcmp(token, "unload\n") == 0) {
    char *c_name = strtok_r(NULL, " ", &saveptr);
    int idx = container_index(*c, c_name);
    unload(&(*c)->f[idx]);
    printf("Arquivo descarregado\n");
  }

  // HEAD
  if (strcmp(token, "head") == 0) {
    char *f_name = strtok_r(NULL, " ", &saveptr);
    int n = atoi(strtok_r(NULL, " ", &saveptr));
    int idx = container_index(*c, f_name);
    if (idx == -1) {
      printf("Erro: %s não encontrado\n", f_name);
      return;
    }

    struct frame **f = &(*c)->f[idx];

    char *out = malloc(64 * (*c)->f[idx]->cols * sizeof(char));
    if (out == NULL) {
      printf("Erro ao alocar memória para out\n");
      return;
    }
    for (int i = 0; i < strlen(out); i++) {
      out[i] = '\0';
    }
    head(&out, *f, n);
    printf("%s\n", out);
  }

  // SUM
  if (strcmp(token, "sum") == 0) {
    char *f_name = strtok_r(NULL, " ", &saveptr);
    char *col = strtok_r(NULL, " ", &saveptr);
    int idx = container_index(*c, f_name);
    if (idx == -1) {
      printf("Erro: %s não encontrado\n", f_name);
      return;
    }

    sum((*c)->f[idx], atoi(col));
  }
}

int main(int argc, char *argv[]) {
  struct container *c = malloc(sizeof(struct container));
  if (c == NULL) {
    return 1;
  }

  c->size = 0;
  c->c_name = (char **)malloc(16 * sizeof(char *));

  char *input = NULL;
  size_t len = 0;

  // Inicio teste
  FILE *tf = fopen("test", "r");
  while (getline(&input, &len, tf) != -1) {
    printf("%s", input);
    handle_input(&input, &c);
  }
  // Fim teste

  while (1) {
    printf("dtst> ");
    getline(&input, &len, stdin);
    handle_input(&input, &c);
  }
  return 0;
}
