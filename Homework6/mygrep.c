#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pattern.h"

/**
   Return true if the given character is ordinary, if it should just
   match occurrences of itself.  This returns false for metacharacters
   like '*' that control how patterns are matched.

   @param c Character that should be evaluated as ordinary or special.
   @return True if c is not special.
*/
static bool ordinary( char c )
{
  // See if c is on our list of special characters.
  if ( strchr( ".^$*?+|()[{", c ) )
    return false;
  return true;
}

/**
  Print the appropriate error message for an invalid pattern and exit unsuccessfully.
*/
static void invalidPattern()
{
  fprintf( stderr, "Invalid pattern\n" );
  exit( EXIT_FAILURE );
}

/**
   Parse regular expression syntax with the highest precedence,
   individual, ordinary symbols, start and end anchors, character
   classes and patterns surrounded by parentheses.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed,
              increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next
           portion of str.
*/
static Pattern *parseAtomicPattern( char *str, int *pos )
{
  if ( ordinary( str[ *pos ] ) )
    return makeSymbolPattern( str[ (*pos)++ ] );

  invalidPattern();
  return NULL; // Just to make the compiler happy.
}

/**
   Parse regular expression syntax with the second-highest precedence,
   a pattern, p, optionally followed by one or more repetition syntax like '*' or '+'.
   If there's no repetition syntax, it just returns the pattern object for p.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed,
              increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next
           portion of str.
*/
static Pattern *parseRepetition( char *str, int *pos )
{
  Pattern *p = parseAtomicPattern( str, pos );
  return p;
}

/**
   Parse regular expression syntax with the third-highest precedence,
   one pattern, p, (optionally) followed by additional patterns
   (concatenation).  If there are no additional patterns, it just
   returns the pattern object for p.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed,
              increased as characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next
           portion of str.
*/
static Pattern *parseConcatenation( char *str, int *pos )
{
  // Parse the first pattern
  Pattern *p1 = parseRepetition( str, pos );
  // While there are additional patterns, parse them
  while ( str[ *pos ] && str[ *pos ] != '|' && str[ *pos ] != ')' ) {
    Pattern *p2 = parseRepetition( str, pos );
    // And build a concatenation pattern to match the sequence.
    p1 = makeConcatenationPattern( p1, p2 );
  }

  return p1;
}

/**
   Parse regular expression syntax with the lowest precedence,
   one pattern, p, (optionally) followed by additional patterns
   separated by | (alternation).  If there are no additional patterns, it just
   returns the pattern object for p.

   @param str The string being parsed.
   @param pos A pass-by-reference value for the location in str being parsed, increased as
              characters from str are parsed.
   @return a dynamically allocated representation of the pattern for the next portion of str.
*/
static Pattern *parseAlternation( char *str, int *pos )
{
  Pattern *p1 = parseConcatenation( str, pos );
  return p1;
}

/**
   A temporary version of main, that just shows a little bit about how
   regular expressions and pattern matching are supposed to work.
*/
int main(int argc, char *argv[])
{
  if (argc > DICT_POS || argc < DICT_POS - 1) {
    fprintf(stderr, "usage: mygrep <pattern> [input-file.txt]\n");
    exit(EXIT_FAILURE);
  }
  if (argc == DICT_POS && fopen(argv[DICT_POS - 1], "r") == NULL) {
    fprintf(stderr, "Can't open input file: %s\n", argv[DICT_POS - 1]);
    exit(EXIT_FAILURE);
  }
  if (argv[1][0] == '*' || argv[1][0] == '+' || argv[1][0] == '|') {
    fprintf(stderr, "Invalid pattern\n");
    exit(EXIT_FAILURE);
  }
  int count = 0;
  for (int i = 0; i < strlen(argv[1]); i++) {
    if (argv[1][i] == '[' || argv[1][i] == '(' || argv[1][i] == '{')
      count++;
    else if (argv[1][i] == ']' || argv[1][i] == ')' || argv[1][i] == '}')
      count--;
  }
  if (count != 0) {
    fprintf(stderr, "Invalid pattern\n");
    exit(EXIT_FAILURE);
  }
  Pattern *pat;

  // Parse a simple pattern.
  int pos = 0;
  pat = parseAlternation( "b", &pos );
  printf( "For pattern 'b'\n" );

  // Try matching this pattern against "abc", I make a new block here
  // so I can let these variables go out of scope when I'm done with them.
  {
    char *str = "abc";

    // before we've matched anything, everywhere in the string is a match.
    bool before[] = { true, true, true, true };
    bool after[ sizeof( before ) / sizeof( before[ 0 ] ) ];

    // Show where the marks are before and after matching.
    printf( "Before matching: " );
    reportMarks( str, before );

    pat->match( pat, strlen( str ), str, before, after );

    printf( "After matching:  " );
    reportMarks( str, after );
  }

  // Try matching against a longer string, with more occurrences of b.
  {
    char *str = "abbbcbbdb";

    bool before[] = { true, true, true, true, true, true, true, true, true, true  };
    bool after[ sizeof( before ) / sizeof( before[ 0 ] ) ];

    printf( "Before matching: " );
    reportMarks( str, before );

    pat->match( pat, strlen( str ), str, before, after );

    printf( "After matching:  " );
    reportMarks( str, after );
  }

  // Done with the first pattern.
  pat->destroy( pat );

  // Try a pattern with concatenation.
  pos = 0;
  pat = parseAlternation( "bc", &pos );
  printf( "For pattern 'bc'\n" );

  {
    char *str = "abcbcdbcb";

    bool before[] = { true, true, true, true, true, true, true, true, true, true  };
    bool after[ sizeof( before ) / sizeof( before[ 0 ] ) ];

    printf( "Before matching: " );
    reportMarks( str, before );

    pat->match( pat, strlen( str ), str, before, after );

    printf( "After matching:  " );
    reportMarks( str, after );
  }

  pat->destroy( pat );

  return EXIT_SUCCESS;
}
