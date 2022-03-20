#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct KV {
  int key;
  char *value;
  struct KV *previous;
  struct KV *next;
};

struct DB {
  int size;
  struct KV *first;
};

struct KV* convertStr2KV(char *string) {
    char *token;
    struct KV *kv = malloc(sizeof(struct KV));
    int index = 0;
    while ((token = strsep(&string, ",")) != NULL) {
      if (index == 0) kv->key = atoi(&token[0]);
      else if (index == 1) kv->value = strdup(token);
      index ++;
    }
    return kv;
};

struct DB* _readDB() {
  struct DB *db = malloc(sizeof(struct DB));
  db->size = 0;

  FILE *fo = fopen("./database.txt", "r");
  char *line = NULL;
  size_t linecap = 0;
  ssize_t linelen;
  if ((linelen = getline(&line, &linecap, fo)) > 0) {
    db->size = atoi(line);
  }

  struct KV *current = NULL;
  for (int i = 0; i < db->size; i++) {
    getline(&line, &linecap, fo);
    // the line adds `\n` at the end automaticaly
    // replace the last character with \0 to replace it.
    line[strlen(line) - 1] = '\0';
    struct KV *next = convertStr2KV(line);
    if (current == NULL) {
      current = db->first = next;
      continue;
    }
    current->next = next;
    next->previous = current;
    current = next;
  }
  fclose(fo);

  return db;
}

struct DB* _updateDBwithKVs(struct DB *db, struct KV *kvs[], int size) {
  db->size += size;

  for (int i = 0; i < size; i++) {
    struct KV *current = db->first;
    struct KV *newKV = kvs[i];

    if (current == NULL) {
      /* printf("add at first position\n"); */
      current = db->first = newKV;
      continue;
    }

    // insert new into db
    while (current != NULL) {
      if (current->key == newKV->key) {
        current->value = newKV->value;
        db->size--;
        break;
      } else if (current->key > newKV->key) {
        /* printf("insert %d in front of %d\n", newKV->key, current->key); */
        if (current->previous == NULL) {
          db->first = newKV;
        } else {
          current->previous->next = newKV;
        }
        newKV->previous = current->previous;
        newKV->next = current;
        current->previous = newKV;
        break;
      } else {
        if (current->next == NULL) {
          /* printf("append new one\n"); */
          current->next = newKV;
          newKV->previous = current;
          break;
        }
        current = current->next;
      }
    }
  }

  return db;
}

struct DB * _deleteKeyFromDB(struct DB *db, int key) {
  struct KV *current = db->first;
  while (current != NULL && current->key < key) {
      current = current->next;
  }

  if (current != NULL && current->key == key) {
    db->size--;
    if (current->previous == NULL) {
      db->first = current->next;
    } else {
      current->previous->next = current->next;
    }

    if (current->next != NULL)
      current->next->previous = current->previous;
    free(current);
  }
  return db;
}

void _writeDB(struct DB *db) {
  FILE *fw = fopen("./database.txt", "w");
  fprintf(fw, "%d\n", db->size);

  struct KV *current = db->first;
  while (NULL != current) {
    fprintf(fw, "%d,%s\n", current->key, current->value);
    /* printf("insert: %d -> %s\n", current->key, current->value); */
    current = current->next;
  }
  fclose(fw);
}

void save(struct KV *kvs[], int size) {
  struct DB *db = _readDB();
  db = _updateDBwithKVs(db, kvs, size);
  _writeDB(db);
}

void get(int key) {
  struct DB *db = _readDB();
  struct KV *current = db->first;
  while (current != NULL) {
    if (current->key == key) {
      printf("%d,%s\n", key, current->value);
      return;
    }
    current = current->next;
  }

  printf("%d not found\n", key);
}

void delete(int key) {
  struct DB *db = _readDB();
  int originalSize = db->size;
  db = _deleteKeyFromDB(db, key);
  if (db->size == originalSize)
    printf("%d not found\n", key);
  else
    _writeDB(db);
}

void all() {
  struct DB *db = _readDB();
  struct KV *current = db->first;
  while (current != NULL) {
    printf("%d,%s\n", current->key, current->value);
    current = current->next;
  }
}

void clear() {
  struct DB *db = malloc(sizeof(struct DB));
  db->size = 0;
  _writeDB(db);
}

int main(int argc, char *argv[]) {

  if (argc <= 1) {
    /* printf("not enough args, need at least 1"); */
    return 0;
  }

  char action, *key, *value;

  char *token, *string, *tofree;
  tofree = string = strdup(argv[1]);

  int index = 0;
  while ((token = strsep(&string, ",")) != NULL) {
    if (index == 0) action = token[0];
    else if (index == 1) key = token;
    else if (index == 2) value = token;
    index ++;
  }

  if (action == 'p') {
      struct KV *kvs[argc-1];
      for (int i = 1; i < argc; i++) {

        tofree = string = strdup(argv[i]);

        int index = 0;
        while ((token = strsep(&string, ",")) != NULL) {
          if (index == 0) {
            action = token[0];
            if (action != 'p') {
              printf("wrong action; only put support multiple arguments");
              return -2;
            }
          }
          else if (index == 1) key = token;
          else if (index == 2) value = token;
          index ++;
        }

        if (key == NULL || value == NULL) {
          printf("missing key or value");
          return -2;
        }
        struct KV *kv = malloc(sizeof(struct KV));
        kv->key = atoi(key);
        kv->value = value;
        /* printf("%d -> %s\n", kv->key, kv->value); */
        kvs[i-1] = kv;
        /* free(kv); */
      }

      save(kvs, argc-1);
      for(int i = 0; i < argc-1; i++) {
        free(kvs[i]);
      }
      return 0;
  }

  switch (action) {
    case 'g': // get
      if (key == NULL) {
        printf("missing key");
        return -2;
      }
      get(atoi(key));
      break;
    case 'a': // get all
      all();
      break;
    case 'c': // clear up
      clear();
      break;
    case 'd': // delete single element
      if (key == NULL) {
        printf("missing key");
        return -2;
      }
      delete(atoi(key));
      break;
    default:
      printf("unsurported action: %c", action);
      return -2;
  }

  free(tofree);

  return 0;
}

