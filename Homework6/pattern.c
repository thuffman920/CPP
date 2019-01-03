#include "pattern.h"
#include <stdlib.h>
#include <stdio.h>

// Documented in the header.
void reportMarks( const char *str, const bool *marks )
{
  int i = 0;
  while ( str[ i ] ) {
    printf( "%c%c", marks[ i ] ? '*' : ' ', str[ i ] );
    i++;
  }

  printf( "%c\n", marks[ i ] ? '*' : ' ' );
}

/**
   A simple function that can be used to free the memory for any
   pattern that doesn't allocate any additional memory other than the
   struct used to represent it (e.g., if it doesn't contain any
   sub-patterns).

   @param pat The pattern to free memory for.
*/
static void destroySimplePattern( Pattern *pat )
{
  // The concrete type of pat doesn't matter.  free() can still
  // free its memory, given a pointer to the start of it.
  free( pat );
}

/**
   Type of pattern used to represent a single, ordinary symbol, like 'a' or '5'.
*/
typedef struct {
  void (*match)( Pattern *pat, int len, const char *str,
                 const bool *before, bool *after );
  void (*destroy)( Pattern *pat );
  
  // Symbol this pattern is supposed to match.
  char sym;
} SymbolPattern;


// Method used to match a SymbolPattern.
static void matchSymbolPattern( Pattern *pat, int len, const char *str,
                                const bool *before, bool *after )
{
  // Cast down to the struct type pat really points to.
  SymbolPattern *this = (SymbolPattern *) pat;

  // If we had a match before an occurrence of this symbol, we now
  // have a match after that occurrence of this symbol.
  after[ 0 ] = false;
  for ( int i = 0; i < len; i++ )
    after[ i + 1 ] = ( before[ i ] && str[ i ] == this->sym );
}

// Documented in the header.
Pattern *makeSymbolPattern( char sym )
{
  // Make an instance of SymbolPattern, and fill in its state.
  SymbolPattern *this = (SymbolPattern *) malloc( sizeof( SymbolPattern ) );
  this->sym = sym;
  
  this->match = matchSymbolPattern;
  this->destroy = destroySimplePattern;

  return (Pattern *) this;
}

/**
   Representation for a type of pattern that just contains two
   sub-patterns (e.g., concatenation).
*/
typedef struct {
  void (*match)( Pattern *pat, int len, const char *str,
                 const bool *before, bool *after );
  void (*destroy)( Pattern *pat );
  
  // Pointers to the two sub-patterns.
  Pattern *p1, *p2;
} BinaryPattern;

// destroy function used for BinaryPattern
static void destroyBinaryPattern( Pattern *pat )
{
  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *) pat;

  // Free our two sub-patterns, then free the struct.
  this->p1->destroy( this->p1 );
  this->p2->destroy( this->p2 );
  free( this );
}

// match function for a BinaryPattern used to handle concatenation.
static void matchConcatenationPattern( Pattern *pat, int len, const char *str,
                                const bool *before, bool *after )
{
  // Cast down to the struct type pat really points to.
  BinaryPattern *this = (BinaryPattern *) pat;

  // Temporary storage for the marks after matching the first sub-pattern.
  bool midMarks[ len + 1 ];
  
  // Match each of the sub-patterns in order.
  this->p1->match( this->p1, len, str, before, midMarks );
  this->p2->match( this->p2, len, str, midMarks, after );
}

// Documented in the header
Pattern *makeConcatenationPattern( Pattern *p1, Pattern *p2 )
{
  // Make an instance of Binary pattern and fill in its fields.
  BinaryPattern *this = (BinaryPattern *) malloc( sizeof( BinaryPattern ) );
  this->p1 = p1;
  this->p2 = p2;
  
  this->match = matchConcatenationPattern;
  this->destroy = destroyBinaryPattern;
  
  return (Pattern *) this;
}
