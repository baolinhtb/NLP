#include "Prob.h"
#include "Ngram.h"
#include "Vocab.h"
#include "File.h"
#include "LM.h"
#include "GenText.h"
#include <unistd.h>
#include "TLSWrapper.h"
#include <cstdio>
#include <cstring>
#include <cmath>
    
#include <iostream>
#include <fstream>


#include "RemoteLM.h"
#include "NgramStats.h"
#include "NBest.h"
#include "Array.cc"

//#include <iostream>

Vocab *swig_srilm_vocab;
const float BIGNEG = -99;
VocabIter *srilm_vocab_iter;

// Initialize the ngram model
Ngram* initLM(int order) {
  swig_srilm_vocab = new Vocab;
  return new Ngram(*swig_srilm_vocab, order);
}

// Delete the ngram model
void deleteLM(Ngram* ngram) {
  delete swig_srilm_vocab;
  delete ngram;
}

// Get index for given string
unsigned getIndexForWord(const char *s) {
  unsigned ans;
  ans = swig_srilm_vocab->addWord((VocabString)s);
  if(ans == Vocab_None) {
    printf("Trying to get index for Vocab_None.\n");
  }
  return ans;
}

// Get the word for a given index
const char* getWordForIndex(unsigned i) {
  return swig_srilm_vocab->getWord((VocabIndex)i);
}

const char* genSentence(Ngram *ngram){
    //VocabString* sent;
    //sent = ngram->generateSentence(50000,(VocabString*)0);
    return ngram->generateSentence(50000,(VocabString*)0)[0];
}



//void ranSentences(Ngram* ngram, unsigned numSentences, const char* filename){
//    VocabString* sent;
//    File outFile(filename,"w");
//    unsigned i;
//    for(i=0;i<numSentences;i++){
//        sent = ngram->generateSentence(50000,(VocabString *) 0);
//        swig_srilm_vocab->write(outFile, sent);
//        fprintf(outFile,"\n");
//    }
//    outFile.close();
//}


// Read in an LM file into the model
int readLM(Ngram* ngram, const char* filename) {
    File file(filename, "r");
    if(file.error()) {
        fprintf(stderr,"Error:: Could not open file %s\n", filename);
        return 0;
    }
    else
        return ngram->read(file, 0);
}

// Get word probability
float getWordProb(Ngram* ngram, unsigned w, unsigned* context) {
    return (float)ngram->wordProb(w, context);
}

// Get word probability accept context
//float getWordProbacc(Ngram* ngram, unsigned w) {
//    unsigned* context;
//    unsigned hist[1] = {Vocab_None};
//    context=hist;
//    return (float)ngram->wordProb(w, context);
//}

// Get unigram probability
float getUnigramProb(Ngram* ngram, const char* word) {
    unsigned index;
    float ans;

    // fill the history array the empty token
    unsigned hist[1] = {Vocab_None};

    // get the index for this word
    index = getIndexForWord(word);

    // Compute word probability
    ans = getWordProb(ngram, index, hist);

    // If the probability is zero, return the constant representing
    // log(0).
    if(ans == LogP_Zero)
        return BIGNEG;

    return ans;
}

// Get bigram probability
float getBigramProb(Ngram* ngram, const char* ngramstr) {
    const char* words[2];
    unsigned indices[2];
    unsigned numparsed;
    char* scp;
    float ans;

    // Create a copy of the input string to be safe
    scp = strdupa(ngramstr);

    // Parse the bigram into the words
    numparsed = Vocab::parseWords(scp, (VocabString *)words, 2);
    if(numparsed != 2) {
        fprintf(stderr, "Error: Given ngram is not a bigram.\n");
        return -1;
    }

    // Add the words to the vocabulary
    swig_srilm_vocab->addWords((VocabString *)words, (VocabIndex *)indices, 2);

    // Fill the history array
    unsigned hist[2] = {indices[0], Vocab_None};

    // Compute the bigram probability
    ans = getWordProb(ngram, indices[1], hist);

    // Return the representation of log(0) if needed
    if(ans == LogP_Zero)
        return BIGNEG;

    return ans;
}

// Get trigram probability
float getTrigramProb(Ngram* ngram, const char* ngramstr) {
    const char* words[6];
    unsigned indices[3];
    unsigned numparsed;
    char* scp;
    float ans;

    // Duplicate
    scp = strdupa(ngramstr);

    numparsed = Vocab::parseWords(scp, (VocabString *)words, 6);
    if(numparsed != 3) {
        fprintf(stderr, "Error: Given ngram is not a trigram.\n");
        return 0;
    }

    swig_srilm_vocab->addWords((VocabString *)words, (VocabIndex *)indices, 3);

    unsigned hist[3] = {indices[1], indices[0], Vocab_None};

    ans = getWordProb(ngram, indices[2], hist);

    if(ans == LogP_Zero)
        return BIGNEG;

    return ans;
}

// get generic n-gram probability (up to n=7)
float getNgramProb(Ngram* ngram, const char* ngramstr, unsigned order) {
    const char* words[7];
    unsigned int indices[order];
    int numparsed, histsize, i, j;
    char* scp;
    float ans;

    // Duplicate string so that we don't mess up the original
    scp = strdupa(ngramstr);

    // Parse the given string into words
    numparsed = Vocab::parseWords(scp, (VocabString *)words, 7);
    if(numparsed != order) {
        fprintf(stderr, "Error: Given order (%d) does not match number of words (%d).\n", order, numparsed);
        return 0;
    }

    // Get indices for the words obtained above, if you don't find them, then add them
    // to the vocabulary and then get the indices.
    swig_srilm_vocab->addWords((VocabString *)words, (VocabIndex *)indices, order);

    // Create a history array of size "order" and populate it
    unsigned hist[order];
    for(i=order; i>1; i--) {
        hist[order-i] = indices[i-2];
    }
    hist[order-1] = Vocab_None;

    // Compute the ngram probability
    ans = getWordProb(ngram, indices[order-1], hist);

    // Return the representation of log(0) if needed
    if(ans == LogP_Zero)
        return BIGNEG;

    return ans;
}

// probability and perplexity at the sentence level
unsigned sentenceStats(Ngram* ngram, const char* sentence, unsigned length, TextStats &stats) {
    float ans;
    // maxWordsPerLine is defined in File.h and so we will reuse it here
    const char* words[maxWordsPerLine + 1];
    unsigned indices[2];
    unsigned numparsed;
    char* scp;

    // Create a copy of the input string to be safe
    scp = strdupa(sentence);

    // Parse the bigram into the words
    numparsed = Vocab::parseWords(scp, (VocabString *)words, maxWordsPerLine + 1);
    if(numparsed != length) {
        fprintf(stderr, "Error: Number of words in sentence does not match given length.\n");
        return 1;
    }
    else {
        ngram->sentenceProb(words, stats);
        return 0;
    }
}


float getSentenceProb(Ngram* ngram, const char* sentence, unsigned length) {
    TextStats stats;
    float ans;

    if(!sentenceStats(ngram, sentence, length, stats)) {
        if (stats.prob == LogP_Zero) {
            ans = BIGNEG;
        }
        else {
            ans = stats.prob;
        }
    }
    else {
        ans = -1.0;
    }

    return ans;
}


float getSentencePpl(Ngram* ngram, const char* sentence, unsigned length) {
    float ans;
    TextStats stats;

    if(!sentenceStats(ngram, sentence, length, stats)) {
        int denom = stats.numWords - stats.numOOVs - stats.zeroProbs + stats.numSentences;
        if (denom > 0) {
            ans = LogPtoPPL(stats.prob / denom);
        }
        else {
            ans = -1.0;
        }
    }
    else {
        ans = -1.0;
    }

    return ans;
}

// how many OOVs in the sentence
int numOOVs(Ngram* ngram, const char* sentence, unsigned length) {
    float ans;
    TextStats stats;

    if(!sentenceStats(ngram, sentence, length, stats)) {
        ans = stats.numOOVs;
    }
    else {
        ans = -1;
    }

    return ans;
}

// probability and perplexity at the corpus level
unsigned corpusStats(Ngram* ngram, const char* filename, TextStats &stats) {
    File corpus(filename, "r");

    if(corpus.error()) {
        fprintf(stderr,"Error:: Could not open file %s\n", filename);
        return 1;
    }
    else {
        ngram->pplFile(corpus, stats, 0);
        return 0;
    }
}


float getCorpusProb(Ngram* ngram, const char* filename) {
    TextStats stats;
    float ans;
    if(!corpusStats(ngram, filename, stats))
        ans = stats.prob;
    else
        ans = -1.0;

    return ans;
}

float getCorpusPpl(Ngram* ngram, const char* filename) {
    TextStats stats;
    float ans;

    if(!corpusStats(ngram, filename, stats)) {
        int denom = stats.numWords - stats.numOOVs - stats.zeroProbs + stats.numSentences;
        if (denom > 0) {
            ans = LogPtoPPL(stats.prob / denom);
        }
        else {
            ans = -1.0;
        }
        return ans;
    }
}


// How many ngrams are in the model
int howManyNgrams(Ngram* ngram, unsigned order) {
  return ngram->numNgrams(order);
}



// Generate random sentences
void ranSentences(Ngram* ngram, unsigned numSentences, const char* filename){
    VocabString* sent;
    File file(filename,"w");
    unsigned i;
    srand48(time(NULL) + getpid());
    for(i=0;i<numSentences;i++){
        sent = ngram->generateSentence(50000,(VocabString *) 0);
        swig_srilm_vocab->write(file, sent);
        fprintf(file,"\n");
    }
    file.close();
}

const char* gensbWord(Ngram* ngram,const char* inputword){
    unsigned index;
    index = getIndexForWord(inputword);
    return srilm_vocab_iter->next(index);
}

static TLSW(unsigned, viDefaultResultSize);
static TLSW(VocabString*, viDefaultResult);

VocabIndex generateWord(const VocabIndex *context)
{
    /*
     * Algorithm: generate random number between 0 and 1, and partition
     * the interval 0..1 into pieces corresponding to the word probs.
     * Choose the word whose interval contains the random value.
     */
    VocabIndex wid = Vocab_None;
    unsigned numtries = 0;
    const unsigned generateMaxTries = 10;

    while (wid == Vocab_None && numtries < generateMaxTries) {
    Prob rval = drand48();
    Prob prob = 0, totalProb = 0;
    VocabIter iter(vocab);
    Boolean first = true;

    while (totalProb <= rval && iter.next(wid)) {
        prob = LogPtoProb(first ? wordProb(wid, context) :
                                  wordProbRecompute(wid, context));
        totalProb += prob;
        first = false;
    }

    /*
     * We've drawn a word that shouldn't have any probability mass.
     * Issue warning and try again.
     */
    if (isNonWord(wid)) {
        if (prob > 0.0 && debug(DEBUG_PRINT_WORD_PROBS)) {
        dout() << "nonword " << vocab.getWord(wid)
               << " has nonzero probability " << prob << endl;
        }
        wid = Vocab_None;
    }
    }

    if (wid == Vocab_None) {
    dout() << "giving up word generation after " << generateMaxTries << endl;
    wid = vocab.seIndex();
    }

    return wid;
}

VocabIndex *genSentence(unsigned maxWords, VocabIndex *sentence, VocabIndex *prefix) {

    Vocab &vocab;
    //if no result buffer is supplied use our own.
    
 
    /*
     * If no result buffer is supplied use our own.
     */
    unsigned &defaultResultSize = TLSW_GET(viDefaultResultSize);
    VocabIndex* &defaultResult  = TLSW_GET(viDefaultResult);

    if (sentence == 0) {
    if (maxWords + 1 > defaultResultSize) {
        defaultResultSize = maxWords + 1;
        if (defaultResult) {
        delete [] defaultResult;
        }
        defaultResult = new VocabIndex[defaultResultSize];
        assert(defaultResult != 0);
    }
    sentence = defaultResult;
    }

    /*
     * Check if a prefix is to be used, and how long it is
     */
    unsigned contextLength;

    if (prefix == 0) {
    if (vocab.ssIndex() != Vocab_None) {
        contextLength = 1;
    } else {
        contextLength = 0;
    }
    } else {
    contextLength = Vocab::length(prefix);
    }

    /*
     * Since we need to add the begin/end sentences tokens, and
     * partial contexts are represented in reverse we use a second
     * buffer for partial sentences.
     */
    unsigned last = maxWords + contextLength;

    makeArray(VocabIndex, genBuffer, last + 1);
    genBuffer[last] = Vocab_None;

    if (prefix == 0) {
        if (contextLength == 1) {
            genBuffer[--last] = vocab.ssIndex();
        }
        } else {
            for (unsigned i = 0; i < contextLength; i ++) {
                genBuffer[--last] = prefix[i];
        }
    }

    /*
     * Generate words one-by-one until hitting an end-of-sentence.
     */
    while (last > 0 && genBuffer[last] != vocab.seIndex()) {
        last --;
        genBuffer[last] = generateWord(&genBuffer[last + 1]);
    }
    
    /*
     * Copy reversed sentence to output buffer
     */
    unsigned i, j;
    for (i = 0, j = maxWords - 1; j > last; i++, j--) {
        sentence[i] = genBuffer[j];
    }
    sentence[i] = Vocab_None;

    return sentence;

}


static TLSW(unsigned, vsDefaultResultSize);
static TLSW(VocabString*, vsDefaultResult);

/*
 * generateSentence and generateWord are non-deterministic when used by multiple
 * threads because of the drand call in generateWord. This could be addressed by 
 * having the caller provide a seed or introducing a TLS seed. The former 
 * approach would provide isolation from other drand calls that may be 
 * introduced. 
 */
VocabString *generateSentence(unsigned maxWords, VocabString *sentence, VocabString *prefix)
{
    unsigned &defaultResultSize = TLSW_GET(vsDefaultResultSize);
    VocabString* &defaultResult  = TLSW_GET(vsDefaultResult);
    /*
     * If no result buffer is supplied use our own.
     */
    if (sentence == 0) {
    if (maxWords + 1 > defaultResultSize) {
        defaultResultSize = maxWords + 1;
        if (defaultResult) {
            delete [] defaultResult;
        }
        defaultResult = new VocabString[defaultResultSize];
        assert(defaultResult != 0);
    }
    sentence = defaultResult;
    }

    VocabIndex *resultIds;

    /*
     * Generate words indices
     */
    if (prefix) {
        unsigned contextLength = Vocab::length(prefix);
        makeArray(VocabIndex, prefixIds, contextLength + 1);

        vocab.getIndices(prefix, prefixIds, contextLength + 1, vocab.unkIndex());
        resultIds = generateSentence(maxWords, (VocabIndex *)0, prefixIds);
    } else {
        resultIds = generateSentence(maxWords, (VocabIndex *)0);
    }

    /*
     * Map them to strings
     */
    vocab.getWords(resultIds, sentence, maxWords + 1);
    return sentence;
}


// Initialize the ngram model
int main(int argc, char *argv[])
{
    Ngram* n;
   // char* varo;
    n= initLM(3);
    readLM(n,"model");
    ranSentences(n,2,"sentences.txt");
    //cin>>varo;
    cout<<getIndexForWord("bathroom");
    cout<<"\n";
   // cout<<gensbWord(n,"bathroom");
    deleteLM(n);
    cout<<"Successful";
    cout<<"\n";
}
