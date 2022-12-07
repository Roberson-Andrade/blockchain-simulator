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

struct BlocoOrdenado
{
  int totalBitcoin;
  unsigned char hashAnterior[SHA256_DIGEST_LENGTH];
  BlocoMinerado *enderecoBlocoMinerado;
  struct BlocoOrdenado *prox;
};

typedef struct BlocoOrdenado BlocoOrdenado;

void imprimeHash(unsigned char hash[], int length);
void imprimeDados(BlocoNaoMinerado *blocoAMinerar);

void minerarBloco();
void calculaTransacoes(BlocoNaoMinerado *blocoAMinerar, MTRand *randNumber);
void preencheHashAnterior(BlocoNaoMinerado *blocoAMinerar);
void mineraBlocos(BlocoMinerado **primeiroBloco);
void validaBloco(BlocoNaoMinerado *blocoAMinerar, BlocoMinerado **primeiroBloco);
void insereBlocoMinerado(BlocoMinerado **primeiroBloco, BlocoNaoMinerado blocoRecemMinerado, unsigned char hash[]);
void imprimeBlocos(BlocoMinerado *primeiroBloco);
void buscaBloco(BlocoMinerado *primeiroBloco, int numeroDoBloco);
int somaTotalBitcoin(BlocoMinerado *primeiroBloco);
void ordenaBlocoEmOrdemCrescente(BlocoMinerado *primeiroBloco, BlocoOrdenado **primeiroBlocoOrdenado);
void insereBlocoOrdenado(BlocoMinerado *blocoAtual, BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual);
void imprimeBlocosOrdenados(BlocoOrdenado *primeiroBlocoOrdenado);
void insereBlocoOrdenadoNoInicio(BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual, BlocoMinerado *blocoAtual);
void insereBlocoOrdenadoDepois(BlocoOrdenado *blocoOrdenadoAtual, int totalBitcoinAtual, BlocoMinerado *blocoAtual);
void insereBlocoOrdenadoFim(BlocoOrdenado *ultimoBloco, int totalBitcoinAtual, BlocoMinerado *blocoAtual);

int main()
{
  BlocoMinerado *primeiroBloco = NULL;
  mineraBlocos(&primeiroBloco);

  int escolha;

  do
  {
    printf("[1] Buscar o hash pelo numero do bloco.\n");
    printf("[2] Listar os blocos por total de bitcoin em ordem crescente.\n");
    printf("[0] Sair\n");
    scanf("%d", &escolha);

    switch (escolha)
    {
    case 1:
    {
      int numeroEscolhido;
      printf("Digite um numero de 1 a 100.000.\n");
      scanf("%d", &numeroEscolhido);
      buscaBloco(primeiroBloco, numeroEscolhido);
      break;
    }

    case 2:
    {
      BlocoOrdenado *primeiroBlocoOrdenado = NULL;

      ordenaBlocoEmOrdemCrescente(primeiroBloco, &primeiroBlocoOrdenado);
      imprimeBlocosOrdenados(primeiroBlocoOrdenado);
      break;
    }

    default:
      printf("Opcao invalida!\n");
    }

  } while (escolha != 0);
  return 0;
}

/**
 * @brief Função que minera todos os blocos
 *
 * @param primeiroBloco
 */
void mineraBlocos(BlocoMinerado **primeiroBloco)
{

  BlocoNaoMinerado blocoAMinerar;
  unsigned char *hashAnterior = NULL;

  MTRand randNumber = seedRand(1234567); // objeto gerador com semente 1234567

  for (int i = 0; i < 100; i++)
  {

    printf("\n\n BLOCO %d - ESTÁ SENDO MINERADO \n\n", i);
    blocoAMinerar.numero = i + 1;

    calculaTransacoes(&blocoAMinerar, &randNumber);
    validaBloco(&blocoAMinerar, primeiroBloco);
  }
}

/**
 * @brief Simula dados das transações para gerar o hash
 *
 * @param blocoAMinerar
 * @param randNumber
 */
void calculaTransacoes(BlocoNaoMinerado *blocoAMinerar, MTRand *randNumber)
{

  unsigned char qtdTransacoes = (unsigned char)(1 + (genRandLong(randNumber) % 61)); // gera aleatorio de 1 a 61

  printf("qtdTransacoes: %d\n", qtdTransacoes);

  for (int i = 0; i < qtdTransacoes; i++)
  {
    unsigned char endOrigem = (unsigned char)genRandLong(randNumber) % 256;
    unsigned char endDst = (unsigned char)genRandLong(randNumber) % 256;
    unsigned char qtdBitcoin = (unsigned char)genRandLong(randNumber) % 50;

    int dataPosition = i * 3;

    blocoAMinerar->data[dataPosition] = endOrigem;
    blocoAMinerar->data[dataPosition + 1] = endDst;
    blocoAMinerar->data[dataPosition + 2] = qtdBitcoin;
  }

  for (int i = qtdTransacoes * 3; i < 184; i++)
  {
    blocoAMinerar->data[i] = 0;
  }
}

/**
 * @brief Função que varia o nounce para encontrar os 2 primeiros zeros e validar o hash
 *
 * @param blocoAMinerar
 * @param primeiroBloco
 */
void validaBloco(BlocoNaoMinerado *blocoAMinerar, BlocoMinerado **primeiroBloco)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];

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

/**
 * @brief Insere o bloco recém minerado na lista encadeada
 *
 * @param primeiroBloco
 * @param blocoRecemMinerado
 * @param hash
 */
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

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    novoBloco->bloco.hashAnterior[i] = (*primeiroBloco)->hash[i];
  }

  novoBloco->prox = *primeiroBloco;
  *primeiroBloco = novoBloco;
}

void imprimeHash(unsigned char hash[], int length)
{
  int i;

  for (i = 0; i < length; ++i)
    printf("%02x", hash[i]);

  printf("\n");
}

void imprimeDados(BlocoNaoMinerado *blocoAMinerar)
{
  size_t dataLength = sizeof(blocoAMinerar->data) / sizeof(blocoAMinerar->data[0]);

  for (int i = 0; i < dataLength; i++)
  {
    printf("%d, ", blocoAMinerar->data[i]);
  }
}

void imprimeBlocosOrdenados(BlocoOrdenado *primeiroBlocoOrdenado)
{
  int count = 1;
  do
  {

    printf("bloco numero: %d; bitcoin: %d;\n", count, primeiroBlocoOrdenado->totalBitcoin);

    primeiroBlocoOrdenado = primeiroBlocoOrdenado->prox;
    count++;
  } while (primeiroBlocoOrdenado != NULL);
}

void imprimeBlocos(BlocoMinerado *primeiroBloco)
{
  do
  {

    printf("%d", primeiroBloco->bloco.numero);

    primeiroBloco = primeiroBloco->prox;
  } while (primeiroBloco != NULL);
}

void buscaBloco(BlocoMinerado *primeiroBloco, int numeroDoBloco)
{
  if (primeiroBloco == NULL)
  {
    printf("\n Bloco nao encontrado \n");
    return;
  }

  if (primeiroBloco->bloco.numero == numeroDoBloco)
  {
    printf("O bloco numero %d tem hash igual a ", numeroDoBloco);
    imprimeHash(primeiroBloco->hash, SHA256_DIGEST_LENGTH);
    printf("\n");
    return;
  }

  buscaBloco(primeiroBloco->prox, numeroDoBloco);
}

int somaTotalBitcoin(BlocoMinerado *primeiroBloco)
{
  int total = 0;

  int index = 0;

  while (primeiroBloco->bloco.data[index] != 0)
  {
    int dataPosition = index * 3;

    total += primeiroBloco->bloco.data[dataPosition + 2];

    index += 1;
  }

  return total;
}
int globaCount = 1;
void ordenaBlocoEmOrdemCrescente(BlocoMinerado *primeiroBloco, BlocoOrdenado **primeiroBlocoOrdenado)
{

  if (primeiroBloco == NULL)
  {
    return;
  }

  int totalBitcoin = somaTotalBitcoin(primeiroBloco);

  insereBlocoOrdenado(primeiroBloco, primeiroBlocoOrdenado, totalBitcoin);

  ordenaBlocoEmOrdemCrescente(primeiroBloco->prox, primeiroBlocoOrdenado);
}

void insereBlocoOrdenadoNoInicio(BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual, BlocoMinerado *blocoAtual)
{
  BlocoOrdenado *novoBlocoOrdenado = malloc(sizeof(BlocoOrdenado));
  novoBlocoOrdenado->prox = *primeiroBlocoOrdenado;
  novoBlocoOrdenado->totalBitcoin = totalBitcoinAtual;
  novoBlocoOrdenado->enderecoBlocoMinerado = blocoAtual;
  *primeiroBlocoOrdenado = novoBlocoOrdenado;
}

void insereBlocoOrdenadoDepois(BlocoOrdenado *blocoOrdenadoAtual, int totalBitcoinAtual, BlocoMinerado *blocoAtual)
{
  BlocoOrdenado *novoBlocoOrdenado = malloc(sizeof(BlocoOrdenado));
  novoBlocoOrdenado->prox = blocoOrdenadoAtual->prox;
  novoBlocoOrdenado->totalBitcoin = totalBitcoinAtual;
  novoBlocoOrdenado->enderecoBlocoMinerado = blocoAtual;

  blocoOrdenadoAtual->prox = novoBlocoOrdenado;
}

void insereBlocoOrdenadoFim(BlocoOrdenado *ultimoBloco, int totalBitcoinAtual, BlocoMinerado *blocoAtual)
{
  BlocoOrdenado *novoBlocoOrdenado = malloc(sizeof(BlocoOrdenado));
  novoBlocoOrdenado->prox = NULL;
  novoBlocoOrdenado->totalBitcoin = totalBitcoinAtual;
  novoBlocoOrdenado->enderecoBlocoMinerado = blocoAtual;

  ultimoBloco->prox = novoBlocoOrdenado;
}

void insereBlocoOrdenado(BlocoMinerado *blocoAtual, BlocoOrdenado **primeiroBlocoOrdenado, int totalBitcoinAtual)
{

  if (*primeiroBlocoOrdenado == NULL || totalBitcoinAtual <= (*primeiroBlocoOrdenado)->totalBitcoin)
  {
    insereBlocoOrdenadoNoInicio(primeiroBlocoOrdenado, totalBitcoinAtual, blocoAtual);
    return;
  }

  BlocoOrdenado *blocoAuxiliar = *primeiroBlocoOrdenado;

  while (blocoAuxiliar->prox != NULL)
  {
    if (totalBitcoinAtual <= blocoAuxiliar->prox->totalBitcoin)
    {
      insereBlocoOrdenadoDepois(blocoAuxiliar, totalBitcoinAtual, blocoAtual);
      return;
    }

    blocoAuxiliar = blocoAuxiliar->prox;
  }

  insereBlocoOrdenadoFim(blocoAuxiliar, totalBitcoinAtual, blocoAtual);
}