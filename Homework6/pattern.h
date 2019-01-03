#define DICT_POS 3
#ifndef _PATTERN_H_
#define _PATTERN_H_

#include <stdbool.h>

//////////////////////////////////////////////////////////////////////
// Superclass for Patterns

/** A short name to use for the Pattern interface. */
typedef struct PatternTag Pattern;

/**
  Structure used as a superclass/interface for patterns.  It includes
  an override-able method for matching against a given string, and for
  destroying itself to free any allocated resources.
*/
struct PatternTag {
  /**
    Pointer to a function to match this pattern against the given
    string.  As described in the design, this function fills in
    locations in the after array to indicate places in the string
    that could be reached after this pattern is matched.  The before
    and after arrays must be one element longer than the length of
    the string being matched.

    @param pat The pattern that's supposed to match itself against the string.
    @param len Length of the string, we could compute it, but it's more efficient to pass it in.
    @param str The input string being matched against.
    @param before Marks for locations in the string that could be reached before matching this
                  pattern.
    @param after Marks for locations in the string that can be reached after matching this
                 pattern.
   */
  void (*match)( Pattern *pat, int len, const char *str,
                 const bool *before, bool *after );

  /**
    Free memory for this pattern, including any subpatterns it contains.
    @param pat pattern to free.
  */
  void (*destroy)( Pattern *pat );
};

/**
  Make a pattern for a single, non-special character, like `a` or `5`.

  @param sym The symbol this pattern is supposed to match.
  @return A dynamically allocated representation for this new pattern.
*/
Pattern *makeSymbolPattern( char sym );

/**
  Make a pattern for the concatenation of patterns p1 and p2.  It should match anything
  that can be broken into two substrings, s1 and s2, where the p1 matches the first
  part (s1) and p2 matches the second part (s2).

  @param p1 Subpattern for matching the first part of the string.
  @param p2 Subpattern for matching the second part of the string.
  @return A dynamically allocated representation for this new pattern.
*/
Pattern *makeConcatenationPattern( Pattern *p1, Pattern *p2 );

/**
  Helpful functiont to print out a string, with the marks shown
  between the characters as asterisks.

  @param str String to report.
  @param marks Array of marks between the characters of str, assumed
         to be one element longer than the length of str.
*/
void reportMarks( const char *str, const bool *marks );

#endif
