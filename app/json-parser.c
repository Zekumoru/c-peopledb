#include "json-parser.h"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
TokenManager* createTokenManager()
{
  TokenManager* manager = (TokenManager*)malloc(sizeof(TokenManager));
  manager->tokens = NULL;
  manager->capacity = 0;
  manager->size = 0;
  manager->pos = 0;
  return manager;
}
void deleteTokenManager(TokenManager* manager)
{
  free(manager->tokens);
  free(manager);
}
Token* createToken(TokenManager* manager)
{
  manager->size++;
  manager->tokens = (Token*)vec_alloc(manager->tokens, &manager->capacity, manager->size, sizeof(Token));
  return &manager->tokens[manager->size - 1];
}
bool matchFileCharacter(FILE* jsonFile, int match, LexError* error, LexErrorType errorType)
{
  int c = fgetc(jsonFile);
  if ((c != match || c == EOF) && error)
  {
    error->type = errorType;
    return false;
  }
  return true;
}
TokenManager* lex(FILE* jsonFile, LexError* error)
{
  fseek(jsonFile, 0, SEEK_SET);
  if (error)
    error->type = NO_LEX_ERROR;
  TokenManager* manager = createTokenManager();
  int c;
  size_t lineCount = 0;
  size_t charCount = 0;
  while ((c = fgetc(jsonFile)) != EOF)
  {
    charCount++;
    if (c == '\n' || c == '\r')
    {
      lineCount++;
      charCount = 0;
      if (c == '\r')
      {
        int next = fgetc(jsonFile);
        if (next != '\n' && next != EOF)
          ungetc(next, jsonFile);
      }
    }
    if (isspace(c))
    {
      continue;
    }
    Token* token = createToken(manager);
    token->startPos = ftell(jsonFile) - 1;
    token->lineCount = lineCount + 1;
    token->charCount = charCount;
    error->lineCount = lineCount + 1;
    error->charCount = charCount;
    switch (c)
    {
    case '{':
    case '}':
    case '[':
    case ']':
    case ',':
    case ':':
      token->type = (TokenType)c;
      token->endPos = token->startPos;
      continue;
    }
    if (c == '"')
    {
      token->type = STRING_LEX;
      do
      {
        c = fgetc(jsonFile);
        charCount++;
      } while (c != '"' && c != EOF);
      if (c == '"')
        token->endPos = ftell(jsonFile) - 1;
      if (c == EOF && error)
      {
        error->type = EXPECTED_END_OF_STRING;
        return manager;
      }
    }
    else if (c == '-' || isdigit(c))
    {
      bool isDouble = false;
      do
      {
        c = fgetc(jsonFile);
        if (c == '.')
          isDouble = true;
        charCount++;
      } while ((isdigit(c) || c == '.') && c != EOF);
      ungetc(c, jsonFile);
      token->endPos = ftell(jsonFile);
      if (isDouble)
        token->type = DOUBLE_LEX;
      else
        token->type = INTEGER_LEX;
      if (c == EOF && error)
      {
        error->type = UNEXPECTED_END_OF_INPUT;
        return manager;
      }
    }
    else if (c == 't')
    {
      token->type = BOOLEAN_LEX;
      if (!matchFileCharacter(jsonFile, 'r', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 'u', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 'e', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      token->endPos = ftell(jsonFile);
      charCount += 3;
    }
    else if (c == 'f')
    {
      token->type = BOOLEAN_LEX;
      if (!matchFileCharacter(jsonFile, 'a', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 'l', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 's', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 'e', error, INVALID_BOOLEAN_LITERAL))
        return manager;
      token->endPos = ftell(jsonFile);
      charCount += 4;
    }
    else if (c == 'n')
    {
      token->type = NULL_LEX;
      if (!matchFileCharacter(jsonFile, 'u', error, INVALID_NULL_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 'l', error, INVALID_NULL_LITERAL))
        return manager;
      if (!matchFileCharacter(jsonFile, 'l', error, INVALID_NULL_LITERAL))
        return manager;
      token->endPos = ftell(jsonFile);
      charCount += 3;
    }
    else
    {
      error->type = UNEXPECTED_CHARACTER;
      return manager;
    }
  }
  if (lineCount == 0 && charCount == 0 && error)
  {
    error->type = EMPTY_FILE;
    error->charCount = 0;
    error->lineCount = 0;
  }
  return manager;
}
JsonNode* createJsonNode(JsonNodeType type)
{
  JsonNode* node = (JsonNode*)malloc(sizeof(JsonNode));
  node->type = type;
  node->key = NULL;
  node->value.v_object = NULL;
  node->isRoot = false;
  node->vCapacity = 0;
  node->vSize = 0;
  return node;
}
Token* advance(TokenManager* manager)
{
  if (manager->pos >= manager->size)
    return NULL;
  Token* token = manager->tokens + manager->pos;
  manager->pos++;
  return token;
}
JsonNode* parse_helper(FILE* jsonFile, TokenManager* manager, ParserError* error)
{
  if (error && error->type != NO_PARSER_ERROR)
    return NULL;
  if (manager->size == 0 || manager->tokens == NULL)
  {
    error->type = NO_TOKEN_FOUND;
    error->token.lineCount = 0;
    error->token.charCount = 0;
    return NULL;
  }
  Token* token = advance(manager);
  if (token == NULL)
    return NULL;
  if (token->type == CURLY_OPEN)
    return parseObject(jsonFile, manager, error);
  if (token->type == BRACKET_OPEN)
    return parseArray(jsonFile, manager, error);
  if (token->type == STRING_LEX)
    return parseString(jsonFile, token);
  if (token->type == INTEGER_LEX)
    return parseInteger(jsonFile, token, error);
  if (token->type == DOUBLE_LEX)
    return parseDouble(jsonFile, token, error);
  if (token->type == BOOLEAN_LEX)
    return parseBoolean(jsonFile, token);
  if (token->type == NULL_LEX)
    return parseNull(jsonFile, token);
  if (error)
  {
    error->type = UNEXPECTED_TOKEN;
    error->token = *token;
  }
  return NULL;
}
JsonNode* parse(FILE* jsonFile, TokenManager* manager, ParserError* error)
{
  if (error)
    error->type = NO_PARSER_ERROR;
  JsonNode* root = parse_helper(jsonFile, manager, error);
  if (root != NULL)
    root->isRoot = true;
  return root;
}
void addObjectPair(JsonNode* node, JsonNode* pairNode)
{
  node->vSize++;
  node->value.v_object = (JsonNode*)vec_alloc(node->value.v_object, &node->vCapacity, node->vSize, sizeof(JsonNode));
  node->value.v_object[node->vSize - 1] = *pairNode;
}
JsonNode* parseObject(FILE* jsonFile, TokenManager* manager, ParserError* error)
{
  JsonNode* node = createJsonNode(OBJECT_NODE);
  Token* token = advance(manager);
  if (token == NULL)
  {
    if (error)
      error->type = EXPECTED_END_OF_OBJECT_BRACE;
    return node;
  }
  if (token->type == CURLY_CLOSE)
    return node;
  manager->pos--;
  while (true)
  {
    token = advance(manager);
    if (token == NULL || token->type != STRING_LEX)
    {
      if (error)
      {
        error->type = EXPECTED_OBJECT_KEY;
        if (token != NULL)
          error->token = *token;
      }
      return node;
    }
    JsonNode* strNode = parseString(jsonFile, token);
    char* pairKey = strNode->value.v_string;
    free(strNode);
    token = advance(manager);
    if (token == NULL || token->type != COLON)
    {
      if (error)
      {
        error->type = EXPECTED_COLON;
        if (token != NULL)
          error->token = *token;
      }
      free(pairKey);
      return node;
    }
    JsonNode* valueNode = parse_helper(jsonFile, manager, error);
    valueNode->key = pairKey;
    if (valueNode == NULL || (error && error->type != NO_PARSER_ERROR))
      return node;
    addObjectPair(node, valueNode);
    free(valueNode);
    token = advance(manager);
    if (token == NULL)
    {
      if (error)
        error->type = EXPECTED_END_OF_OBJECT_BRACE;
      return node;
    }
    if (token->type == CURLY_CLOSE)
      return node;
    if (token->type != COMMA)
    {
      if (error)
      {
        error->type = EXPECTED_COMMA;
        error->token = *token;
      }
      return node;
    }
  }
  if (error)
  {
    error->type = EXPECTED_END_OF_OBJECT_BRACE;
    error->token = *token;
  }
  return node;
}
void addElement(JsonNode* node, JsonNode* elemNode)
{
  node->vSize++;
  node->value.v_array = (JsonNode*)vec_alloc(node->value.v_array, &node->vCapacity, node->vSize, sizeof(JsonNode));
  node->value.v_array[node->vSize - 1] = *elemNode;
}
JsonNode* parseArray(FILE* jsonFile, TokenManager* manager, ParserError* error)
{
  JsonNode* node = createJsonNode(ARRAY_NODE);
  Token* token = advance(manager);
  if (token == NULL)
  {
    if (error)
      error->type = EXPECTED_END_OF_ARRAY_BRACE;
    return node;
  }
  if (token->type == BRACKET_CLOSE)
    return node;
  manager->pos--;
  while (true)
  {
    JsonNode* elemNode = parse_helper(jsonFile, manager, error);
    if (elemNode == NULL && error && error->type != NO_PARSER_ERROR)
      return node;
    addElement(node, elemNode);
    free(elemNode);
    token = advance(manager);
    if (token == NULL)
    {
      if (error)
        error->type = EXPECTED_END_OF_ARRAY_BRACE;
      return node;
    }
    if (token->type == BRACKET_CLOSE)
      return node;
    if (token->type != COMMA)
    {
      if (error)
      {
        error->type = EXPECTED_COMMA;
        error->token = *token;
      }
      return node;
    }
  }
  if (error)
  {
    error->type = EXPECTED_END_OF_ARRAY_BRACE;
    error->token = *token;
  }
  return node;
}
char* getStringFromToken(FILE* jsonFile, Token* token)
{
  size_t strLength = token->endPos - token->startPos;
  size_t startPos = token->startPos;
  if (token->type != STRING_LEX)
    strLength++;
  else
    startPos++;
  char* str = (char*)malloc(strLength);
  fseek(jsonFile, startPos, SEEK_SET);
  fgets(str, strLength, jsonFile);
  return str;
}
JsonNode* parseString(FILE* jsonFile, Token* token)
{
  JsonNode* node = createJsonNode(STRING_NODE);
  node->value.v_string = getStringFromToken(jsonFile, token);
  return node;
}
JsonNode* parseInteger(FILE* jsonFile, Token* token, ParserError* error)
{
  JsonNode* node = createJsonNode(INTEGER_NODE);
  char* input = getStringFromToken(jsonFile, token);
  char* endptr;
  node->value.v_int = (int)strtol(input, &endptr, 10);
  if (*endptr != '\0' && error)
  {
    error->type = INVALID_INTEGER_LITERAL;
    error->token = *token;
  }
  free(input);
  return node;
}
JsonNode* parseDouble(FILE* jsonFile, Token* token, ParserError* error)
{
  JsonNode* node = createJsonNode(DOUBLE_NODE);
  char* input = getStringFromToken(jsonFile, token);
  char* endptr;
  node->value.v_double = strtod(input, &endptr);
  if (*endptr != '\0' && error)
  {
    error->type = INVALID_DOUBLE_LITERAL;
    error->token = *token;
  }
  free(input);
  return node;
}
JsonNode* parseBoolean(FILE* jsonFile, Token* token)
{
  JsonNode* node = createJsonNode(BOOLEAN_NODE);
  fseek(jsonFile, token->startPos, SEEK_SET);
  int c = fgetc(jsonFile);
  node->value.v_bool = (c == 't');
  return node;
}
JsonNode* parseNull(FILE* jsonFile, Token* token)
{
  JsonNode* node = createJsonNode(NULL_NODE);
  return node;
}
void freeJsonTree(JsonNode* node)
{
  if (node == NULL)
    return;
  if (node->key != NULL)
    free(node->key);
  switch (node->type)
  {
  case NULL_NODE:
  case INTEGER_NODE:
  case DOUBLE_NODE:
  case BOOLEAN_NODE:
    break;
  case STRING_NODE:
    free(node->value.v_string);
    break;
  case OBJECT_NODE:
  case ARRAY_NODE:
    JsonNode* nodeList;
    if (node->type == OBJECT_NODE)
    {
      nodeList = node->value.v_object;
    }
    else
    {
      nodeList = node->value.v_array;
    }
    for (size_t i = 0; i < node->vSize; i++)
      freeJsonTree(&nodeList[i]);
    free(nodeList);
    break;
  }
  if (node->isRoot)
    free(node);
}
void* vec_alloc(void* vec, size_t* cap, const size_t size, const size_t elemSize)
{
  if (size == 0 || elemSize == 0)
    return vec;
  *cap = pow(2, floor(log2(size)) + 1);
  void* newVec = realloc(vec, *cap * elemSize);
  if (newVec == NULL)
    free(vec);
  return newVec;
}
void printError(const char* errorType, size_t lineCount, size_t charCount, const char* message)
{
  printf("Error: %s at line %ld, column %ld: %s\n", errorType, lineCount, charCount, message);
}
void printLexError(LexError* error)
{
  switch (error->type)
  {
  case EMPTY_FILE:
    printError("Syntax Error", error->lineCount, error->charCount, "Expected JSON content");
    break;
  case EXPECTED_END_OF_STRING:
    printError("Syntax Error", error->lineCount, error->charCount, "Expected end-of-string double quotes");
    break;
  case INVALID_BOOLEAN_LITERAL:
    printError("Syntax Error", error->lineCount, error->charCount, "Invalid Boolean literal");
    break;
  case INVALID_NULL_LITERAL:
    printError("Syntax Error", error->lineCount, error->charCount, "Invalid Boolean literal");
    break;
  case UNEXPECTED_END_OF_INPUT:
    printError("Syntax Error", error->lineCount, error->charCount, "Unexpected end-of-input token");
    break;
  case UNEXPECTED_CHARACTER:
    printError("Syntax Error", error->lineCount, error->charCount, "Unexpected character");
    break;
  }
}
void printParseError(ParserError* error)
{
  switch (error->type)
  {
  case NO_TOKEN_FOUND:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Expected token but none found");
    break;
  case INVALID_INTEGER_LITERAL:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Invalid integer literal");
    break;
  case INVALID_DOUBLE_LITERAL:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Invalid double literal");
    break;
  case EXPECTED_OBJECT_KEY:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Expected object key");
    break;
  case EXPECTED_END_OF_OBJECT_BRACE:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Expected end-of-object brace");
    break;
  case EXPECTED_END_OF_ARRAY_BRACE:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Expected end-of-array brace");
    break;
  case EXPECTED_COLON:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Expected colon after object key");
    break;
  case EXPECTED_COMMA:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Expected comma");
    break;
  case UNEXPECTED_TOKEN:
    printError("Syntax Error", error->token.lineCount, error->token.charCount, "Unexpected token");
    break;
  }
}
void printTokens(TokenManager* manager)
{
  for (size_t i = 0; i < manager->size; i++)
  {
    const Token* token = &manager->tokens[i];
    printf("Type: ");
    switch (token->type)
    {
    case NULL_LEX:
      printf("NULL_LEX");
      break;
    case STRING_LEX:
      printf("STRING_LEX");
      break;
    case INTEGER_LEX:
      printf("INTEGER_LEX");
      break;
    case DOUBLE_LEX:
      printf("DOUBLE_LEX");
      break;
    case BOOLEAN_LEX:
      printf("BOOLEAN_LEX");
      break;
    case CURLY_OPEN:
      printf("CURLY_OPEN");
      break;
    case CURLY_CLOSE:
      printf("CURLY_CLOSE");
      break;
    case BRACKET_OPEN:
      printf("BRACKET_OPEN");
      break;
    case BRACKET_CLOSE:
      printf("BRACKET_CLOSE");
      break;
    case COMMA:
      printf("COMMA");
      break;
    case COLON:
      printf("COLON");
      break;
    }
    printf("\n");
  }
}
void printWithIndent(size_t indent, const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  for (size_t i = 0; i < indent; i++)
    printf(" ");
  vprintf(fmt, args);
  va_end(args);
}
void traverse(JsonNode* node, size_t indent, bool isParentArray)
{
  if (node == NULL)
    return;
  switch (node->type)
  {
  case NULL_NODE:
    printWithIndent(indent, "- ");
    if (!isParentArray)
      printf("%s: ", node->key);
    printf("(null)\n");
    break;
  case STRING_NODE:
    printWithIndent(indent, "- ");
    if (!isParentArray)
      printf("%s: ", node->key);
    printf("%s\n", node->value.v_string);
    break;
  case INTEGER_NODE:
    printWithIndent(indent, "- ");
    if (!isParentArray)
      printf("%s: ", node->key);
    printf("%d\n", node->value.v_int);
    break;
  case DOUBLE_NODE:
    printWithIndent(indent, "- ");
    if (!isParentArray)
      printf("%s: ", node->key);
    printf("%lf\n", node->value.v_double);
    break;
  case BOOLEAN_NODE:
    printWithIndent(indent, "- ");
    if (!isParentArray)
      printf("%s: ", node->key);
    printf("%s\n", node->value.v_bool ? "true" : "false");
    break;
  case OBJECT_NODE:
  case ARRAY_NODE:
  {
    JsonNode* nodeList;
    bool hasNoKey = node->key == NULL;
    if (node->type == OBJECT_NODE)
    {
      if (!hasNoKey)
        printWithIndent(indent, "- %s:", node->key);
      nodeList = node->value.v_object;
      if (node->vSize == 0)
        printf(" {}");
      if (!hasNoKey)
        printf("\n");
    }
    else
    {
      if (!(node->isRoot || hasNoKey))
        printWithIndent(indent, "- %s:", node->key);
      nodeList = node->value.v_array;
      if (node->vSize == 0)
        printf(" []");
      if (!hasNoKey)
        printf("\n");
    }
    size_t indentAdd = hasNoKey ? 0 : 2;
    if (!node->isRoot && hasNoKey && node->type == ARRAY_NODE)
      indentAdd += 2;
    for (size_t i = 0; i < node->vSize; i++)
      traverse(&nodeList[i], indent + indentAdd, node->type == ARRAY_NODE);
    break;
  }
  default:
    printWithIndent(indent, "- [[UNKNOWN NODE]]\n");
  }
}
