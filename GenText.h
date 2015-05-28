#ifndef GENTEXT_H
#define GENTEXT_H

#ifdef PRE_ISO_CXX
# include <iostream.h>
#else
# include <iostream>
using namespace std;
#endif

#include "Boolean.h"
#include "Prob.h"
#include "Counts.h"
#include "File.h"
#include "Vocab.h"
#include "SubVocab.h"
#include "TextStats.h"
#include "Debug.h"
#include "MemStats.h"
#include "NgramStats.h"
    
#ifdef __cplusplus
  extern "C" {
#else
    typedef struct Ngram Ngram; /* dummy type to stand in for class */
#endif

Ngram* initLM(int order);
void deleteLM(Ngram* ngram);
unsigned getIndexForWord(const char* s);
const char* getWordForIndex(unsigned i);
int readLM(Ngram* ngram, const char* filename);
float getWordProb(Ngram* ngram, unsigned word, unsigned* context);
void ranSentences(Ngram* ngram, unsigned numSentences, const char* filename);
const char* gensbWord(Ngram* ngram,const char* inputword);
float getNgramProb(Ngram* ngram, const char* ngramstr, unsigned order);
float getUnigramProb(Ngram* ngram, const char* word);
float getBigramProb(Ngram* ngram, const char* ngramstr);
float getTrigramProb(Ngram* ngram, const char* ngramstr);
float getSentenceProb(Ngram* ngram, const char* sentence, unsigned length);
float getSentencePpl(Ngram* ngram, const char* sentence, unsigned length);
int numOOVs(Ngram* ngram, const char* sentence, unsigned length);
unsigned corpusStats(Ngram* ngram, const char* filename, TextStats &stats);
float getCorpusProb(Ngram* ngram, const char* filename);
float getCorpusPpl(Ngram* ngram, const char* filename);
int howManyNgrams(Ngram* ngram, unsigned order);


#ifdef __cplusplus
  }
#endif

#endif

