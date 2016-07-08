#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>


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

   Assuming that all the parties speak the truth at all
   times, what are these numbers?


   This program is a Computational solution of the above
   Freudenthal Problem. Read two related articles on
   this subject on my website:
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
      ./cfreudenthal 2 99

   The program outputs the corresponding product/sum
   survivors of the consecutive rounds of elimination
   followed by the final answer(s)

 */


/*
   A malloc()ed array of num_t's where 'num'
   is a sum forms matrix's columns' header

   A malloc()ed array of num_t's where 'num'
   is a product forms matrix's rows' header
 */
typedef struct
{
	int		num;
	char		live; /* meaning: not eliminated */
} num_t;

typedef struct
{
	int		minInt;
	int		maxInt;
	int		minSum;
	int		maxSum;

	int		nCols; /* is the number of legal sums */
	int		nRows; /* is the number of legal products */
	num_t*		cols; /* array of legal sums, see above */
	num_t*		rows; /* array of legal products, see above */

	/*
	   Dynamically allocated 2D array of binary values:
	   1 - means that the factors of this product add to this sum
	   0 - means no correlation between this product and this sum
	 */
	char*		matrix; /* dynamically allocated 2D array */
} fr_t;


static void		rmSumsWithUniqueProduct( fr_t* );
static void		rmProductsWithMultipleSums( fr_t* );
static void		rmSumsWithMultipleProducts( fr_t* );
static int		nSums( fr_t*, int, char, int* );
static int		nLiveProducts( fr_t*, int, int* );

static fr_t*		init( int, char* [], fr_t* );
static void		mkSums( fr_t* );
static int		mkProducts( fr_t* );
static void		rmDupProducts( fr_t* );
static void		mkMatrix( fr_t* );
static void		printFr( fr_t* );
static int		cmpNums( const void*, const void* );
static void		printAnswers( fr_t* );
static void		getXY( fr_t*, int, int, int*, int* );


extern int
main( int argc, char* argv[] )
{
	fr_t*		rv;
	fr_t		fr = { 0 };
	int		ec = 0;


	rv = init( argc, argv, &fr );
	if ( !rv )
	{
		ec = 1;
		goto out;
	}

	printf( "Initial matrix:\n" );
	printFr( &fr );

	rmSumsWithUniqueProduct( &fr );

	printf( "\nSurvivors of \"S1: I knew that\":\n" );
	printFr( &fr );

	rmProductsWithMultipleSums( &fr );

	printf( "\nSurvivors of \"P2: But then I know\":\n" );
	printFr( &fr );

	rmSumsWithMultipleProducts( &fr );

	printf( "\nSurvivors of \"S2: And so do I\":\n" );
	printFr( &fr );

	printf( "\nAnswer(s):\n" );
	printAnswers( &fr );

out:
	if ( fr.cols )
	{
		free( fr.cols );
	}

	if ( fr.rows )
	{
		free( fr.rows );
	}

	if ( fr.matrix )
	{
		free( fr.matrix );
	}

	return ec;
}


/*
   For each product (row):

   if a current product (row) has only one
   sum associated with it - eliminate the
   column corresponding to that sum in its
   entirty
 */
static void
rmSumsWithUniqueProduct( fr_t* fr )
{
	int		row;
	int		nsums;
	int		thisColumn;
	char		all = 0;


	for ( row = 0; row < fr->nRows; row++ )
	{
		nsums = nSums( fr, row, all, &thisColumn );

		if ( nsums != 1 )
		{
			continue;
		}

		fr->cols[ thisColumn ].live = 0;
	}
}


/*
   For each product (row):

   keep the current product (row) iff it has
   exactly one live sum associated with it,
   eliminate that row (product) in its entirty
   otherwise
 */
static void
rmProductsWithMultipleSums( fr_t* fr )
{
	int		row;
	int		nsums;
	char		liveOnly = 1;


	for ( row = 0; row < fr->nRows; row++ )
	{
		nsums = nSums( fr, row, liveOnly, NULL );

		if ( nsums == 1 )
		{
			continue;
		}

		fr->rows[ row ].live = 0;
	}
}


/*
   For each live sum (column):

   keep the current live sum (column) iff it
   has exactly one live product associated
   with it, eliminate that column (sum) in
   its entirty otherwise
 */
static void
rmSumsWithMultipleProducts( fr_t* fr )
{
	int		col;
	int		nproducts;


	for ( col = 0; col < fr->nCols; col++ )
	{
		if ( !fr->cols[ col ].live )
		{
			continue;
		}

		nproducts = nLiveProducts( fr, col, NULL );

		if ( nproducts == 1 )
		{
			continue;
		}

		fr->cols[ col ].live = 0;
	}
}


/*
   Compute the number of sums,
   optionally live only, in the
   given row
 */
static int
nSums( fr_t* fr, int row, char liveOnly, int* thisColumn )
{
	char		v;
	int		col;
	int		nsums = 0;


	for ( col = 0; col < fr->nCols; col++ )
	{
		if ( liveOnly && !fr->cols[ col ].live )
		{
			continue;
		}

		v = *( fr->matrix + row * fr->nCols + col );
		if ( v != 1 )
		{
			continue;
		}

		nsums++;
		if ( thisColumn )
		{
			*thisColumn = col;
		}
	}

	return nsums;
}


/*
   Compute the number of live
   products in the given column
 */
static int
nLiveProducts( fr_t* fr, int col, int* thisRow )
{
	char		v;
	int		row;
	int		nproducts = 0;


	for ( row = 0; row < fr->nRows; row++ )
	{
		if ( !fr->rows[ row ].live )
		{
			continue;
		}

		v = *( fr->matrix + row * fr->nCols + col );
		if ( v != 1 )
		{
			continue;
		}

		nproducts++;
		if ( thisRow )
		{
			*thisRow = row;
		}
	}

	return nproducts;
}


static void
printAnswers( fr_t* fr )
{
	int		x;
	int		y;
	int		row;
	int		col;
	int		product;
	int		sum;
	int		nsums;
	char		liveOnly = 1;


	for ( row = 0; row < fr->nRows; row++ )
	{
		if ( !fr->rows[ row ].live )
		{
			continue;
		}

		nsums = nSums( fr, row, liveOnly, &col );

		if ( nsums != 1 )
		{
			continue;
		}

		product = fr->rows[ row ].num;
		sum = fr->cols[ col ].num;
		getXY( fr, product, sum, &x, &y );
		printf( "product = %d, sum = %d, x = %d, y = %d\n",
			product, sum, x, y );
	}
}


/*
   We can find 'x' and 'y' by:
  
   - decomposing the 'sum' into two terms and
   finding those that multiply to 'product'

   - decomposing the 'product' into two factors
   and finding those that sum to 'sum'

   we will use the sum method since that does
   not require the square root calculation

 */
static void
getXY( fr_t* fr, int product, int sum, int* x, int* y )
{
	int		a;
	int		b;
	int		half = sum / 2;


	*x = *y = 0;

	for ( a = fr->minInt; a <= half; a++ )
	{
		b = sum - a;
		if ( ( a * b ) == product )
		{
			*x = a;
			*y = b;
			return;
		}
	}
}


static fr_t*
init( int argc, char* argv[], fr_t* fr )
{
	int		n;


	if ( argc < 3 )
	{
		return NULL;
	}

	fr->minInt = atoi( argv[ 1 ] );
	if ( fr->minInt <= 0 )
	{
		return NULL;
	}

	fr->minSum = fr->minInt + fr->minInt;
	fr->maxSum = atoi( argv[ 2 ] );
	if ( fr->maxSum <= fr->minSum )
	{
		return NULL;
	}

	fr->maxInt = fr->maxSum - fr->minInt;


	fr->nCols = fr->maxSum - fr->minSum + 1;
	fr->cols = ( num_t* )calloc( fr->nCols, sizeof( num_t ) );
	if ( !fr->cols )
	{
		return NULL;
	}
	mkSums( fr );


	/*
	   Compute the upper bound for
	   the number of legal products,
	   including duplicates
	 */
	fr->nRows = mkProducts( fr );

	fr->rows = ( num_t* )calloc( fr->nRows, sizeof( num_t ) );
	if ( !fr->rows )
	{
		return NULL;
	}

	/*
	   Populate the actual products,
	   including duplicates
	 */
	fr->nRows = mkProducts( fr );

	rmDupProducts( fr );


	n = fr->nCols * fr->nRows;
	fr->matrix = ( char* )calloc( n, sizeof( char ) );
	if ( !fr->matrix )
	{
		return NULL;
	}

	mkMatrix( fr );

	return fr;
}


static void
mkSums( fr_t* fr )
{
	int		sum;
	int		col;


	/*
	   The array of sums is automatically
	   sorted in the ascending order
	 */
	for ( sum = fr->minSum, col = 0; sum <= fr->maxSum; sum++, col++ )
	{
		fr->cols[ col ].num = sum;
		fr->cols[ col ].live = 1;
	}
}


static int
mkProducts( fr_t* fr )
{
	int		a;
	int		b;
	int		sum;
	int		prod;
	int		nprods;
	int		ubound = 0;
	int		actual = 0;


	for ( a = fr->minInt; a <= fr->maxInt; a++ )
	{
		for ( b = a + 1; b <= fr->maxInt; b++ )
		{
			sum = a + b;

			if ( sum > fr->maxSum )
			{
				continue;
			}

			if ( !fr->rows )
			{
				ubound++;
				continue;
			}

			prod = a * b;
			fr->rows[ actual ].num = prod;
			fr->rows[ actual ].live = 1;
			actual++;
		}
	}

	if ( !fr->rows )
	{
		nprods = ubound;
	}
	else
	{
		nprods = actual;
	}

	return nprods;
}


static void
rmDupProducts( fr_t* fr )
{
	num_t*		found;
	num_t		key;
	int		row;
	size_t		n;


	qsort( fr->rows, fr->nRows, sizeof( num_t ), cmpNums );

	for ( row = 0; row < fr->nRows; row++ )
	{
		key.num = fr->rows[ row ].num;

		while ( 1 )
		{
			found = bsearch( &key,
				fr->rows + row + 1,
				fr->nRows - ( row + 1 ),
				sizeof( num_t ),
				cmpNums );
			if ( !found )
			{
				break;
			}

			/*
			   Found duplicate is moved into
			   the place of the current product
			 */
			n = ( fr->nRows - ( row + 1 ) ) * sizeof( num_t );
			memmove( fr->rows + row, found, n );
			fr->nRows--;
		}
	}
}


/*
   For each product (row):

   decompose the current product into all
   possible pairs of factors, for each
   pair of factors:

      set the value in the matrix's cell
      (in the current row) in the column
      corresponding to the sum to which
      these two factor add to to 1
 */
static void
mkMatrix( fr_t* fr )
{
	int		a;
	int		b;
	int		col;
	int		row;
	int		product;
	int		sqroot;
	int		sum;
	num_t		key;
	num_t*		found;


	for ( row = 0; row < fr->nRows; row++ )
	{
		product = fr->rows[ row ].num;
		sqroot = ( int )sqrt( ( double )product );

		for( a = 2; a <= sqroot; a++ )
		{
			if ( product % a != 0 )
			{
				continue;
			}

			b = product / a;
			sum = a + b;
			key.num = sum;
			found = bsearch( &key, fr->cols, fr->nCols,
				sizeof( num_t ), cmpNums );
			if ( !found )
			{
				continue;
			}

			col = found - fr->cols;
			*( fr->matrix + row * fr->nCols + col ) = 1;
		}
	}
}


static void
printFr( fr_t* fr )
{
	char		v;
	int		row;
	int		col;
	int		nsums;
	char		liveOnly = 1;


	printf( "minInt = %d, maxInt = %d\n"
		"minSum = %d, maxSum = %d\n"
		"nCols = %d, nRows = %d\n",
		fr->minInt, fr->maxInt,
		fr->minSum, fr->maxSum,
		fr->nCols, fr->nRows );


	printf( "\t" );
	for ( col = 0; col < fr->nCols; col++ )
	{
		if ( !fr->cols[ col ].live )
		{
			continue;
		}

		printf( "%d\t", fr->cols[ col ] );
	}
	printf( "\n" );


	for ( row = 0; row < fr->nRows; row++ )
	{
		if ( !fr->rows[ row ].live )
		{
			continue;
		}

		nsums = nSums( fr, row, liveOnly, NULL );
		if ( nsums < 1 )
		{
			continue;
		}

		printf( "%d\t", fr->rows[ row ].num );

		for ( col = 0; col < fr->nCols; col++ )
		{
			if ( !fr->cols[ col ].live )
			{
				continue;
			}

			v = *( fr->matrix + row * fr->nCols + col );
			printf( "%c\t",
				v == 1 ? '1' : ' ' );
		}
		printf( "\n" );
	}
}


static int
cmpNums( const void* n1, const void* n2 )
{
	num_t*		num1 = ( num_t* )n1;
	num_t*		num2 = ( num_t* )n2;


	if ( num1->num < num2->num )
	{
		return -1;
	}
	else if ( num1->num == num2->num )
	{
		return 0;
	}

	return 1;
}
