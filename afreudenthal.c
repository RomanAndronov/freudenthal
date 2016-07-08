#include <stdio.h>
#include <stdlib.h>
#include <math.h>


/*
   Freudenthal Problem:

   Professor R tells two students, S(um) and P(roduct):

   - I have two non-equal natural numbers in mind: both
   are greater than 1 and their sum is less than 100.
   To S I will whisper the sum of these two numbers so
   that P can not hear it (and R does) and to P I will
   whisper the product of these two numbers so that S
   can not hear it (and R does).

   Later on the following dialog between S and P ensues:

   P1: I can not name these numbers

   S1: I knew that

   P2: But then I can!

   S2: And so do I!

   Assuming that all the parties speak the truth at all times,
   what are these numbers?

   This program is a computational rendering of an Analytic
   solution of the above Freudenthal Problem. Read two
   related articles on this subject on my website:
      romanyandronov.elementfx.com
        Basic Approaches
           Elimination
              Omnia Mea Mecum Porto 1,2

   argv[ 1 ] is the numbers' lower bound

   argv[ 2 ] is the sum's upper bound

   The program's input is inclusive - all the tests to
   generate the relevant sets of numbers are
   "greater/less than or equal to"

   A sample input for a classical Freudenthal Problem
   is:
      ./afreudenthal 2 99

   The program outputs the pairs of numbers along with
   the corresponding product/sum survivors of the
   consecutive statements made by P and S

 */


typedef struct
{
	int		x;
	int		y;
	int		sum;
	int		prod;

	int		prodpp1; /* this product passes P1 */
	int		sumps1; /* this sum passes S1 */
	int		prodpp2; /* this product passes P2 */
	int		sumps2; /* this sum passes S2 */
} fr_t;


static void		checkP1( fr_t*, int );
static int		prodPassesP1( int );

static void		checkS1( fr_t*, int );
static int		sumPassesS1( int );

static void		checkP2( fr_t*, int, int, int );
static int		prodPassesP2( int, int, int );

static void		checkS2( fr_t*, int, int, int );
static int		sumPassesS2( int, int, int );

static int		isPrime( int n );

static fr_t*		init( int, char* [], int*, int*, int* );
static int		mkPairs( fr_t*, int, int );
static void		printFrRow( fr_t* );
static void		printFr( fr_t*, int );


extern int
main( int argc, char* argv[] )
{
	fr_t*		fr;
	int		N = 0;
	int		minInt = 0;
	int		maxInt = 0;
	int		sumUBound = 0;


	fr = init( argc, argv, &minInt, &sumUBound, &N );
	if ( !fr )
	{
		return 0;
	}

	maxInt = sumUBound - minInt;

	printFr( fr, N );

	checkP1( fr, N );

	checkS1( fr, N );

	checkP2( fr, N, minInt, maxInt );

	checkS2( fr, N, minInt, maxInt );

	free( fr );

	return 0;
}


/*
   P announces P1 = "I do not know"

   meaning: the number whispered to P does not
   have a unique pair of factors

   Step through all the Freudenthal pairs of
   numbers and select only those whose product
   has more than one pair of factors (set the
   'prodpp1' member to 1)

 */
static void
checkP1( fr_t* fr, int n )
{
	int		i;


	printf( "[Begin Products That Pass P1:\n" );
	for ( i = 0; i < n; i++ )
	{
		fr[ i ].prodpp1 = prodPassesP1( fr[ i ].prod );
		if ( fr[ i ].prodpp1 )
		{
			printFrRow( &fr[ i ] );
		}
	}
	printf( ":end Products That Pass P1]\n" );
}


/*
   If, for a given product 'p', there exist
   integers 'a' and 'b' such that:

      'a' * 'b' = 'p' and
      'a' and 'b' are both prime

   then return 0, otherwise - return 1

 */
static int
prodPassesP1( int product )
{
	int		a;
	int		b;
	int		sqroot = ( int )sqrt( ( double )product );


	for( a = 2; a <= sqroot; a++ )
	{
		if ( product % a == 0 )
		{
			b = product / a;
			if ( isPrime( a ) && isPrime( b ) )
			{
				return 0;
			}
		}
	}

	return 1;
}


/*
   S announces S1 = "I know that P does not know"

   meaning: the number whispered to S is not
   representable as a sum of two primes - at least
   one term in a two-term decomposition of the sum
   must be composite

   Step through all the pairs of numbers that passed
   P1 and select only those whose sum has at least
   one composite term (set the 'sumps1' to 1)

 */
static void
checkS1( fr_t* fr, int n )
{
	int		i;


	printf( "[Begin Sums That Pass S1:\n" );
	for ( i = 0; i < n; i++ )
	{
		if ( !fr[ i ].prodpp1 )
		{
			continue;
		}

		fr[ i ].sumps1 = sumPassesS1( fr[ i ].sum );
		if ( fr[ i ].sumps1 )
		{
			printFrRow( &fr[ i ] );
		}
	}
	printf( ":end Sums That Pass S1]\n" );
}


/*
   If, for a given sum 's', there exist
   integers 'a' and 'b' such that:

      'a' + 'b' = 's' and
      'a' and 'b' are both prime

   then return 0, otherwise - return 1

 */
static int
sumPassesS1( int sum )
{
	int		a;
	int		b;
	int		half = sum / 2;


	for( a = 2; a <= half; a++ )
	{
		b = sum - a;
		if ( isPrime( a ) && isPrime( b ) )
		{
			return 0;
		}
	}

	return 1;
}


/*
   P announces P2 = "I know"

   meaning: for P to be able to announce P2,
   (s)he must have been whispered a number 'p'
   that has multiple pairs of factors of which
   exactly one pair sums to a number that passes
   S1

   Step through all the pairs of numbers that
   passed P1 and S1 and select only those whose
   product has exactly one pair of factors that
   sums to a number that passes S1 (set the
   'prodpp2' to 1)

 */
static void
checkP2( fr_t* fr, int n, int minint, int maxint )
{
	int		i;


	printf( "[Begin Products That Pass P2:\n" );
	for ( i = 0; i < n; i++ )
	{
		if ( !fr[ i ].prodpp1 || !fr[ i ].sumps1 )
		{
			continue;
		}

		fr[ i ].prodpp2 = prodPassesP2( fr[ i ].prod, minint, maxint );
		if ( fr[ i ].prodpp2 )
		{
			printFrRow( &fr[ i ] );
		}
	}
	printf( ":end Products That Pass P2]\n" );
}


/*
   If, for a given product 'p', among all the
   pairs of integers 'a' and 'b' such that:

      'a' * 'b' = 'p'
      'minint' <= 'a', 'b' <= 'maxint'

   one and only one pair sums to a number
   that passes S1 then return 1, otherwise -
   return 0

 */
static int
prodPassesP2( int product, int minint, int maxint )
{
	int		a;
	int		b;
	int		sum;
	int		cnt = 0;
	int		sqroot = ( int )sqrt( ( double )product );


	for( a = 2; a <= sqroot; a++ )
	{
		if ( product % a == 0 )
		{
			b = product / a;
			if ( b < minint || b > maxint )
			{
				continue;
			}

			sum = a + b;
			if ( sumPassesS1( sum ) )
			{
				if ( cnt > 0 )
				{
					return 0;
				}
				cnt = 1;
			}
		}
	}

	return cnt;
}


/*
   S announces S2 = "I know too"

   meaning: for S to be able to announce S2,
   (s)he must have been whispered a number 's'
   that has multiple pairs of terms that sum to
   's' of which exactly one pair multiplies to a
   product that passes P2

   Step through all the pairs of numbers that
   passed P1, S1 and P2 and select only those
   whose sum has exactly one pair of terms that
   multiply to a product that passes P2 (set the
   'sumps2' to 1)

 */
static void
checkS2( fr_t* fr, int n, int minint, int maxint )
{
	int		i;


	printf( "[Begin Sums That Pass S2:\n" );
	for ( i = 0; i < n; i++ )
	{
		if ( !fr[ i ].prodpp1 || !fr[ i ].sumps1 || !fr[ i ].prodpp2 )
		{
			continue;
		}

		fr[ i ].sumps2 = sumPassesS2( fr[ i ].sum, minint, maxint );
		if ( fr[ i ].sumps2 )
		{
			printFrRow( &fr[ i ] );
		}
	}
	printf( ":end Sums That Pass S2]\n" );
}


/*
   If, for a given sum 's', among all the
   pairs of integers 'a' and 'b' such that:

      'a' + 'b' = 's'
      'minint' <= 'a', 'b' <= 'maxint'

   one and only one pair multiplies to a
   product that passes P2 then return 1,
   otherwise - return 0

 */
static int
sumPassesS2( int sum, int minint, int maxint )
{
	int		a;
	int		b;
	int		prod;
	int		cnt = 0;
	int		half = sum / 2;


	for( a = 2; a <= half; a++ )
	{
		b = sum - a;
		prod = a * b;
		if ( prodPassesP2( prod, minint, maxint ) )
		{
			if ( cnt > 0 )
			{
				return 0;
			}
			cnt = 1;
		}
	}

	return cnt;
}


static fr_t*
init( int argc, char* argv[], int* minint, int* sumubound, int* n )
{
	int		x;
	int		y;
	fr_t*		fr;


	if ( argc < 3 )
	{
		return NULL;
	}

	*minint = atoi( argv[ 1 ] );
	*sumubound = atoi( argv[ 2 ] );


	/*
	   Compute the number of pairs first
	 */
	*n = mkPairs( NULL, *minint, *sumubound );
	if ( *n == 0 )
	{
		return NULL;
	}

	fr = ( fr_t* )calloc( *n, sizeof( fr_t ) );
	if ( !fr )
	{
		return NULL;
	}


	/*
	   Populate them next
	 */
	mkPairs( fr, *minint, *sumubound );

	return fr;
}


static int
mkPairs( fr_t* fr, int minint, int sumubound )
{
	int		x;
	int		y;
	int		n = 0;


	for ( x = minint; x <= sumubound; x++ )
	{
		for ( y = x + 1; y <= sumubound; y++ )
		{
			if ( ( x + y ) <= sumubound  )
			{
				if ( fr )
				{
					fr[ n ].x = x;
					fr[ n ].y = y;
					fr[ n ].sum = x + y;
					fr[ n ].prod = x * y;
				}
				n++;
			}
		}
	}

	return n;
}


static int
isPrime( int n )
{
	int		i;
	int		sqroot;


	if ( n <= 3 )
	{
		return 1;
	}

	if ( n % 2 == 0 )
	{
		return 0;
	}

	sqroot = ( int )sqrt( ( double )n );

	for ( i = 2; i <= sqroot; i++ )
	{
		if ( n % i == 0 )
		{
			return 0;
		}
	}

	return 1;
}


static void
printFrRow( fr_t* row )
{
	printf( "%d %d"
		"\t%d\t%d"
		"\tprodpp1 = %d"
		"\t\t sumps1 = %d"
		"\tprodp2 = %d"
		"\tsumps2 = %d\n",
			row->x, row->y,
			row->sum, row->prod,
			row->prodpp1,
			row->sumps1,
			row->prodpp2,
			row->sumps2 );
}


static void
printFr( fr_t* fr, int n )
{
	int		i;


	printf( "Total of %d Freudenthal pairs:\n", n );
	for ( i = 0; i < n; i++ )
	{
		printFrRow( &fr[ i ] );
	}
}
