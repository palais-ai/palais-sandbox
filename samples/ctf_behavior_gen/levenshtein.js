
  function forEach( array, fn ) { var i, length
    i = -1
    length = array.length
    while ( ++i < length )
      fn( array[ i ], i, array )
  }

  function map( array, fn ) { var result
    result = Array( array.length )
    forEach( array, function ( val, i, array ) {
      result[i] = fn( val, i, array )
    })
    return result
  }

  function reduce( array, fn, accumulator ) {
    forEach( array, function( val, i, array ) {
      accumulator = fn( val, i, array )
    })
    return accumulator
  }

  // Levenshtein distance
  function Levenshtein( str_m, str_n ) { var previous, current, matrix
    // Constructor
    matrix = this._matrix = []

    // Sanity checks
    if ( str_m == str_n )
      return this.distance = 0
    else if ( str_m == '' )
      return this.distance = str_n.length
    else if ( str_n == '' )
      return this.distance = str_m.length
    else {
      // Danger Will Robinson
      previous = [ 0 ]
      forEach( str_m, function( v, i ) { i++, previous[ i ] = i } )

      matrix[0] = previous
      forEach( str_n, function( n_val, n_idx ) {
        current = [ ++n_idx ]
        forEach( str_m, function( m_val, m_idx ) {
          m_idx++
          if ( str_m.charAt( m_idx - 1 ) == str_n.charAt( n_idx - 1 ) )
            current[ m_idx ] = previous[ m_idx - 1 ]
          else
            current[ m_idx ] = Math.min
              ( previous[ m_idx ]     + 1   // Deletion
              , current[  m_idx - 1 ] + 1   // Insertion
              , previous[ m_idx - 1 ] + 1   // Subtraction
              )
        })
        previous = current
        matrix[ matrix.length ] = previous
      })

      return this.distance = current[ current.length - 1 ]
    }
  }

  Levenshtein.prototype.toString = Levenshtein.prototype.inspect = function inspect ( no_print ) { var matrix, max, buff, sep, rows
    matrix = this.getMatrix()
    max = reduce( matrix,function( m, o ) {
      return Math.max( m, reduce( o, Math.max, 0 ) )
    }, 0 )
    buff = Array( ( max + '' ).length ).join( ' ' )

    sep = []
    while ( sep.length < (matrix[0] && matrix[0].length || 0) )
      sep[ sep.length ] = Array( buff.length + 1 ).join( '-' )
    sep = sep.join( '-+' ) + '-'

    rows = map( matrix, function( row ) { var cells
      cells = map( row, function( cell ) {
        return ( buff + cell ).slice( - buff.length )
      })
      return cells.join( ' |' ) + ' '
    })

    return rows.join( "\n" + sep + "\n" )
  }

  Levenshtein.prototype.getMatrix = function () {
    return this._matrix.slice()
  }

  Levenshtein.prototype.valueOf = function() {
    return this.distance
  }

/**
  BSD 2-clause

  Copyright (c) 2012, John Roepke
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

  // TheSpanishInquisition
  
  // Cache the matrix. Note this implementation is limited to
  // strings of 64 char or less. This could be altered to update
  // dynamically, or a larger value could be used.
  var matrix = [];
  for (var i = 0; i < 64; i++) {
      matrix[i] = [i];
      matrix[i].length = 64;
  }
  for (var i = 0; i < 64; i++) {
      matrix[0][i] = i;
  }

  // Functional implementation of Levenshtein Distance.
  String.damerauLevenshteinDistance = function(__this, that, limit) {
        var thisLength = __this.length, thatLength = that.length;
    
        if (Math.abs(thisLength - thatLength) > (limit || 32)) return limit || 32;
        if (thisLength === 0) return thatLength;
        if (thatLength === 0) return thisLength;
    
        // Calculate matrix.
        var this_i, that_j, cost, min, t;
        for (i = 1; i <= thisLength; ++i) {
                this_i = __this[i-1];
    
                // Step 4
                for (j = 1; j <= thatLength; ++j) {
                        // Check the jagged ld total so far
                        if (i === j && matrix[i][j] > 4) return thisLength;
    
                        that_j = that[j-1];
                        cost = (this_i === that_j) ? 0 : 1; // Step 5
                        // Calculate the minimum (much faster than Math.min(...)).
                        min    = matrix[i - 1][j    ] + 1;                                              // Deletion.
                        if ((t = matrix[i    ][j - 1] + 1   ) < min) min = t;   // Insertion.
                        if ((t = matrix[i - 1][j - 1] + cost) < min) min = t;   // Substitution.
    
    
                        // Damerau transposition.
                        if (i > 1 && j > 1 && this_i === that[j-2] && this[i-2] === that_j) {
                                matrix[i][j] = Math.min(matrix[i    ][j    ],
                                                        matrix[i - 2][j - 2] + cost);   // Transposition.
                        }
    
                        matrix[i][j] = min;     // Update matrix.
                }
        }
    
        return matrix[thisLength][thatLength];
    };