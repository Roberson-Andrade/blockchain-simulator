#include "openssl/crypto.h" //arquivo de definição necessário para SHA256_DIGEST_LENGTH
#include "openssl/sha.h"    //arquivo de definição necessário função SHA256
#include <string.h>
#include <stdio.h>
#include "mtwister.c"

int NUMERO_TOTAL_DE_BLOCOS = 100000;

struct BlocoNaoMinerado
{
  unsigned int numero;                              // 4
  unsigned int nonce;                               // 4
  unsigned char data[184];                          // nao alterar. Deve ser inicializado com zeros.
  unsigned char hashAnterior[SHA256_DIGEST_LENGTH]; // 32
};
typedef struct BlocoNaoMinerado BlocoNaoMinerado;

struct BlocoMinerado
{
  BlocoNaoMinerado bloco;
  unsigned char hash[SHA256_DIGEST_LENGTH]; // 32 bytes
  struct BlocoMinerado *prox;
};
typedef struct BlocoMinerado BlocoMinerado;

void printHash(unsigned char hash[], int length);
void printData(BlocoNaoMinerado *blocoAMinerar);

void minerarBloco();
void calculaTransacoes(BlocoNaoMinerado *blocoAMinerar);
void preencheHashAnterior(BlocoNaoMinerado *blocoAMinerar);
void mineraBlocos(BlocoMinerado **primeiroBloco);
void validaBloco(BlocoNaoMinerado *blocoAMinerar, BlocoMinerado **primeiroBloco);
void insereBlocoMinerado(BlocoMinerado **primeiroBloco, BlocoNaoMinerado blocoRecemMinerado, unsigned char hash[]);

int main()
{
  BlocoMinerado *primeiroBloco = NULL;
  mineraBlocos(&primeiroBloco);
  return 0;
}

void printHash(unsigned char hash[], int length)
{
  int i;

  for (i = 0; i < length; ++i)
    printf("%02x", hash[i]);

  printf("\n");
  printf("Primeiros valores: %d %d\n", hash[0], hash[1]);
}

void calculaTransacoes(BlocoNaoMinerado *blocoAMinerar)
{
  MTRand randNumber = seedRand(1234567);                                              // objeto gerador com semente 1234567
  unsigned char qtdTransacoes = (unsigned char)(1 + (genRandLong(&randNumber) % 61)); // gera aleatorio de 1 a 61

  printf("qtdTransacoes: %d\n", qtdTransacoes);

  for (int i = 0; i < qtdTransacoes; i++)
  {
    unsigned char endOrigem = (unsigned char)genRandLong(&randNumber) % 256;
    unsigned char endDst = (unsigned char)genRandLong(&randNumber) % 256;
    unsigned char qtdBitcoin = (unsigned char)genRandLong(&randNumber) % 50;

    int dataPosition = i * 3;

    blocoAMinerar->data[dataPosition] = endOrigem;
    blocoAMinerar->data[dataPosition + 1] = endDst;
    blocoAMinerar->data[dataPosition + 2] = qtdBitcoin;
  }

  for (int i = qtdTransacoes * 3; i < 184; i++)
  {
    blocoAMinerar->data[i] = 0;
  }

  printData(blocoAMinerar);
}

void printData(BlocoNaoMinerado *blocoAMinerar)
{
  size_t dataLength = sizeof(blocoAMinerar->data) / sizeof(blocoAMinerar->data[0]);

  for (int i = 0; i < dataLength; i++)
  {
    printf("%d, ", blocoAMinerar->data[i]);
  }
}

void printBlocos(BlocoMinerado *primeiroBloco)
{
  do
  {

    printf("%d", primeiroBloco->bloco.numero);

    primeiroBloco = primeiroBloco->prox;
  } while (primeiroBloco != NULL);
}

void validaBloco(BlocoNaoMinerado *blocoAMinerar, BlocoMinerado **primeiroBloco)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  // unsigned char *hash = malloc(sizeof(unsigned char) * SHA256_DIGEST_LENGTH);
  // vetor que armazenará o resultado do hash. Tamanho definidio pela libssl

  int nounceCount = 0;

  do
  {
    blocoAMinerar->nonce = nounceCount;

    SHA256((unsigned char *)blocoAMinerar, sizeof(blocoAMinerar), hash);

    nounceCount++;
  } while (hash[0] != 0 || hash[1] != 0);
  printf("Primeiros valores: %d %d\n", hash[0], hash[1]);
  printf("\n\n\n SUCESSO \n\n\n");

  insereBlocoMinerado(primeiroBloco, *blocoAMinerar, hash);
}

void mineraBlocos(BlocoMinerado **primeiroBloco)
{

  BlocoNaoMinerado blocoAMinerar;
  unsigned char *hashAnterior = NULL;

  for (int i = 0; i < 100; i++)
  {

    printf("\n\n BLOCO %d - ESTÁ SENDO MINERADO \n\n", i);
    blocoAMinerar.numero = i;

    // for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    // {
    //   if (hashAnterior == NULL)
    //   {
    //     blocoAMinerar.hashAnterior[i] = 0;
    //   }
    //   else
    //   {
    //     blocoAMinerar.hashAnterior[i] = hashAnterior[i];
    //   }
    // }

    calculaTransacoes(&blocoAMinerar);
    validaBloco(&blocoAMinerar, primeiroBloco);
  }

  printBlocos(*primeiroBloco);
}

void insereBlocoMinerado(BlocoMinerado **primeiroBloco, BlocoNaoMinerado blocoRecemMinerado, unsigned char hash[])
{
  BlocoMinerado *novoBloco = malloc(sizeof(BlocoMinerado));

  novoBloco->bloco = blocoRecemMinerado;

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    novoBloco->hash[i] = hash[i];
  }

  if (*primeiroBloco == NULL)
  {
    novoBloco->prox = NULL;

    *primeiroBloco = novoBloco;
    return;
  }

  novoBloco->prox = *primeiroBloco;
  *primeiroBloco = novoBloco;
}
