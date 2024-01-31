// Following https://news.ycombinator.com/item?id=13082825
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#define nelem(x) (sizeof(x) / sizeof(*(x)))

/****************************************************
 *
 * BEGIN FORTH
 *
 ****************************************************/
#define STACK_SIZE 100
void *stack[STACK_SIZE];
void **stackPointer = stack;

void printStack(void) {
  int totalStackDepth = stackPointer - stack;

  printf("<%d>", totalStackDepth);
  for (int index = 0; index < totalStackDepth; index++) {
    printf(" %d", *((int *)stack[index]));
  }
}

void *popParameter(void) {
  if (stackPointer <= stack)
    errx(EX_DATAERR, "Stack underflow");
  return *(--stackPointer);
}

void pushParameter(void *item) {
  if (stackPointer - stack >= STACK_SIZE)
    errx(EX_DATAERR, "Stack overflow");
  *stackPointer = item;
  stackPointer++;
}

void add(void) {
  int *b = (int *)popParameter();
  int *a = (int *)popParameter();
  *b = *a + *b;
  pushParameter(b);
  free(a);
}

void sub(void) {
  int *b = (int *)popParameter();
  int *a = (int *)popParameter();
  *b = *a - *b;
  pushParameter(b);
  free(a);
}

void print(void) {
  int *a = (int *)popParameter();
  printf("%d", *a);
  free(a);
}

struct DictEntry {
  char *word;
  void (*func)(void);
} dict[] = {
    {"+", add}, {"-", sub}, {".", print}, {".s", printStack}
    /* , {"x", mul}, */
    /* {"/", div}, {"clr", clr} */
};

struct DictEntry *findDictionaryEntry(char *word) {
  struct DictEntry *ptr = dict;
  int difference = ptr - dict;
  // Would this break on another architecture because of different signedness?
  int number = nelem(dict);
  while (difference <= number) {
    if (strcmp(ptr->word, word) == 0) {
      return ptr;
    }
    ptr++;

    difference = ptr - dict;
  }
  return NULL;
}

struct Token {
  char *start;
  size_t length;
} token;

// Returns -1 if nothing to parse
int getNextToken(char *input, int length, struct Token *token) {
  if (token->start == NULL) {
    token->start = input;
  }

  // Move forward to the next non-whitespace
  while (*token->start <= 32 && token->start - input < length) {
    token->start++;
  }

  // If we fell off the end
  if (token->start - input == length) {
    return -1;
  }

  token->length = 1;
  // Now scan to the following whitespace
  while (*(token->start + token->length) > 32 &&
         (token->start + token->length) - input < length) {
    token->length++;
  }

  // Now if we reach the end, it's fine
  return 0;
}

void resetToken(struct Token *token) {
  token->start = NULL;
  token->length = 0;
}

// Return -1 if unable to convert
int numberWordToNumber(char *word, int *number) {
  char *i = word;
  *number = 0;

  while (i != NULL) {
    if (*i < '0' || *i > '9')
      return -1;
    *number *= 10;
    *number += *i - '0';
    i++;
  }
  return 0;
}

void mainLoop(char *input, size_t length) {
  char *word = NULL;
  struct DictEntry *dictEntry;
  int *number = NULL;
  while (getNextToken(input, length, &token) != -1) {
    // If token is in dictionary
    word = malloc(token.length + 1);
    word[token.length] = 0x00;
    strncpy(word, token.start, token.length);
    dictEntry = findDictionaryEntry(word);

    if (dictEntry != NULL) {
      dictEntry->func();
    } else {
      number = malloc(sizeof(int));
      if (numberWordToNumber(word, number)) {
        pushParameter(number);
      } else {
        // TODO Error!
      }
    }
    token.start = token.start + token.length;
  }

  resetToken(&token);
}

/****************************************************
 *
 * END FORTH
 *
 ****************************************************/

/****************************************************
 *
 * BEGIN TESTS
 *
 ****************************************************/

char *input = "2 3 .s 4 + . .s";

int main(void) {
  int *i = malloc(sizeof(int));
  *i = 5;
  pushParameter(i);
  int *p0 = *stack;
  printf("\nParameter[0] = %d\n", *p0);
  if ((char **)stackPointer - (char **)stack >= STACK_SIZE)
    errx(EX_DATAERR, "Stack overflow %ld %p %p", stackPointer - stack,
         (void *)stackPointer, (void *)stack);

  mainLoop(input, strlen(input));

  printf(" Main loop returned\n");
  printStack();
  input = "2 3 .s 4 + . .s";
  mainLoop(input, strlen(input));
  printStack();
}

/****************************************************
 *
 * END TESTS
 *
 ****************************************************/
