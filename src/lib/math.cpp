/*************************************************************

	LSD 9.0 - January 2024
	written by Marco Valente, Universita' dell'Aquila
	and by Marcelo Pereira, University of Campinas

	Copyright Marco Valente and Marcelo Pereira
	LSD is distributed under the GNU General Public License

	See Readme.txt for copyright information of
	third parties' code used in LSD

 *************************************************************/

/*************************************************************
MATH.CPP
Contains the mathematical and statistical functions used in
LSD and models.
*************************************************************/

#include "lib/libLSD.h"				// LSD library classes


/****************************************************
IS_FINITE
function redefinition to handle GCC standard library bugs
****************************************************/
bool is_finite( double x )
{
#if __GNUC__ > 3
	return __builtin_isfinite( x );
#else
	return isfinite( x );
#endif
}


/****************************************************
IS_INF
function redefinition to handle GCC standard library bugs
****************************************************/
bool is_inf( double x )
{
#if __GNUC__ > 3
	return __builtin_isinf( x );
#else
	return isinf( x );
#endif
}


/****************************************************
IS_NAN
function redefinition to handle GCC standard library bugs
****************************************************/
bool is_nan( double x )
{
#if __GNUC__ > 3
	return __builtin_isnan( x );
#else
	return isnan( x );
#endif
}


/****************************************************
_ABS
****************************************************/
double simulation::_abs( double a )
{
	if ( a > 0 )
		return a;
	else
		return ( -1 * a );
};


/****************************************************
ROUND
****************************************************/
double round( double x )
{
	if ( ( x - floor( x ) ) > ( ceil( x ) - x ) )
		return ceil( x );

	return floor( x );
}


/****************************************************
ROUND_DIGITS
****************************************************/
double round_digits( double value, int digits )
{
	if ( value == 0.0 )
		return 0.0;

	double factor = pow( 10.0, digits - ceil( log10( fabs( value ) ) ) );

	return round( value * factor ) / factor;
}


/****************************************************
MAX
****************************************************/
double max( double a, double b )
{
	if ( a > b )
		return a;
	return b;
}


/****************************************************
MIN
****************************************************/
double min ( double a, double b )
{
	if ( a < b )
		return a;
	return b;
}


/****************************************************
LOWER_BOUND
****************************************************/
double lower_bound( double a, double b, double marg, double marg_eq, int dig )
{
	double rmin = round_digits( a, dig );
	double rmax = round_digits( b, dig );

	if ( rmin > rmax )
	{
		double temp = rmin;
		rmin = rmax;
		rmax = temp;
	}

	if ( rmin == rmax )
	{
		if ( rmin == 0.0 )
			return round_digits( - marg_eq, dig );
		else
			if ( rmin > 0 )
				return round_digits( rmin * ( 1 - marg_eq ), dig );
			else
				return round_digits( rmin * ( 1 + marg_eq ), dig );
	}

	if ( rmin == 0.0 )
		return round_digits( - marg, dig );
	else
		if ( rmin > 0 )
			return round_digits( rmin * ( 1 - marg ), dig );
		else
			return round_digits( rmin * ( 1 + marg ), dig );
}


/****************************************************
UPPER_BOUND
****************************************************/
double upper_bound( double a, double b, double marg, double marg_eq, int dig )
{
	double rmin = round_digits( a, dig );
	double rmax = round_digits( b, dig );

	if ( rmin > rmax )
	{
		double temp = rmin;
		rmin = rmax;
		rmax = temp;
	}

	if ( rmin == rmax )
	{
		if ( rmax == 0.0 )
			return round_digits( marg_eq, dig );
		else
			if ( rmax > 0 )
				return round_digits( rmax * ( 1 + marg_eq ), dig );
			else
				return round_digits( rmax * ( 1 - marg_eq ), dig );
	}

	if ( rmax == 0.0 )
		return round_digits( marg, dig );
	else
		if ( rmax > 0 )
			return round_digits( rmax * ( 1 + marg ), dig );
		else
			return round_digits( rmax * ( 1 - marg ), dig );
}


/***************************************************
IPOW
Integer exponentiation
***************************************************/
double ipow( double base, double exp )
{
	long res = 1, lbase = ( long ) floor( base ), lexp = ( long ) floor( exp );

	if ( lexp < 0 )
		return 0;

	while ( true )
	{
		if ( ( lexp & 1 ) != 0 )
			res *= lbase;

		lexp >>= 1;

		if ( lexp == 0 )
			break;

		lbase *= lbase;
	}

	return res;
}


/***************************************************
FACT
Factorial function
***************************************************/
double fact( double x )
{
	x = floor( x );
	if ( x < 0.0 )
	{
		plog( "\nWarning: bad x in function: fact" );
		return 0.0;
	}

	double fact = 1.0;
	long i = 1;
	while (i <= x)
		fact *= i++;

	return fact;
}


/****************************************************
MEDIAN
****************************************************/
double median( vector < double > & v )
{
	int mid;
	double midVal;

	if ( v.empty( ) )
		return NAN;

	mid = v.size( ) / 2;

	auto midPos = v.begin( ) + mid;
	nth_element( v.begin( ), midPos, v.end( ) );
	midVal = v[ mid ];

	if ( v.size( ) % 2 != 0 )
		return midVal;
	else
		return ( * max_element( v.begin( ), midPos ) + midVal ) / 2;
}


/****************************************************
T_STAR
Student t distribution  statistic for given
degrees of freedom and confidence level (in %)
****************************************************/
double t_star( int df, double cl )
{
	int i;

	for ( i = 0; i < T_CLEVS - 1; ++i )
		if ( cl <= 100 * t_dist_cl[ i ] )
			break;

	if ( df <= 30 )
		return t_dist_st[ i ][ df - 1 ];

	if ( df <= 40 )
		return t_dist_st[ i ][ 30 ];

	if ( df <= 60 )
		return t_dist_st[ i ][ 31 ];

	if ( df <= 80 )
		return t_dist_st[ i ][ 32 ];

	if ( df <= 100 )
		return t_dist_st[ i ][ 33 ];

	if ( df <= 1000 )
		return t_dist_st[ i ][ 34 ];

	return t_dist_st[ i ][ 35 ];
}


/****************************************************
Z_STAR
Standard normal distribution statistic for given
confidence level (in %)
****************************************************/
double z_star( double cl )
{
	int i;

	for ( i = 0; i < Z_CLEVS - 1; ++i )
		if ( cl <= 100 * z_dist_cl[ i ] )
			break;

	return z_dist_st[ i ];
}


/***************************************************
UNIFCDF
Uniform cumulative distribution function
***************************************************/
double simulation::unifcdf( double a, double b, double x )
{
	if ( a >= b )
	{
		plog( "\nWarning: bad a or b in function: uniformcdf" );
		return 0.0;
	}

	if ( x <= a )
		return 0.0;
	if ( x >= b )
		return 1.0;

	return ( x - a ) / ( b - a );
}


/***************************************************
POISSONCDF
Poisson cumulative distribution function
***************************************************/
double simulation::poissoncdf( double lambda, double k )
{
	k = floor( k );
	if ( lambda <= 0.0 || k < 0.0 )
	{
		plog( "\nWarning: bad lambda or k in function: poissoncdf" );
		return 0.0;
	}

	double sum = 0.0;
	long i;
	for ( i = 0; i <= k; i++ )
		sum += pow( lambda, i ) / fact( i );

	return exp( -lambda ) * sum;
}


/***************************************************
PARETOCDF
Pareto cumulative distribution function
***************************************************/
double simulation::paretocdf( double mu, double alpha, double x )
{
	if ( mu <= 0 || alpha <= 0 )
	{
		plog( "\nWarning: bad mu, alpha in function: paretocdf" );
		return 0.0;
	}

	if ( x < mu )
		return 0.0;
	else
		return 1.0 - pow( mu / x, alpha );
}


/***************************************************
BPARETOCDF
Bounded Pareto cumulative distribution function
***************************************************/
double simulation::bparetocdf( double alpha, double low, double high, double x )
{
	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		plog( "\nWarning: bad alpha, low or high in function: bparetocdf" );
		return 0.0;
	}

	if ( x < low )
		return 0.0;
	else
		return ( 1 - pow( low, alpha ) * pow( x, - alpha ) ) /
			   ( 1 - pow( low / high, alpha ) ) ;
}


/***************************************************
NORMCDF
Normal cumulative distribution function
***************************************************/
double simulation::normcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 )
	{
		plog( "\nWarning: bad sigma in function: normalcdf" );
		return 0.0;
	}

	return 0.5 * ( 1 + erf( ( x - mu ) / ( sigma * sqrt( 2.0 ) ) ) );
}


/***************************************************
LNORMCDF
Lognormal cumulative distribution function
***************************************************/
double simulation::lnormcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 || x <= 0.0 )
	{
		plog( "\nWarning: bad sigma or x in function: lnormalcdf" );
		return 0.0;
	}

	return 0.5 + 0.5 * erf( ( log( x ) - mu ) / ( sigma * sqrt( 2.0 ) ) );
}


/***************************************************
ALAPLCDF
Asymmetric laplace cumulative distribution function
***************************************************/
double simulation::alaplcdf( double mu, double alpha1, double alpha2, double x )
{
	if ( alpha1 <= 0.0 || alpha2 <= 0.0 )
	{
		plog( "\nWarning: bad alpha in function: alaplcdf" );
		return 0.0;
	}

	if ( x < mu )									// cdf up to upper bound
		return 0.5 * exp( ( x - mu ) / alpha1 );
	else
		return 1 - 0.5 * exp( -( x - mu ) / alpha2 );
}


/***************************************************
BETACF
Beta distribution: continued fraction evaluation function
Press et al. (1992) Numerical Recipes in C, 2nd Ed.
***************************************************/
#define MAXIT 100
#define BEPS 3.0e-7
#define FPMIN 1.0e-30

double simulation::betacf( double a, double b, double x )
{
	void nrerror(char error_text[ ]);
	int m, m2;
	double aa, c, d, del, h, qab, qam, qap;

	qab = a + b;
	qap = a + 1.0;
	qam = a - 1.0;
	c = 1.0;
	d = 1.0 - qab * x / qap;

	if ( fabs( d ) < FPMIN )
		d = FPMIN;
	d = 1.0 / d;
	h = d;

	for ( m = 1; m <= MAXIT; m++ )
	{
		m2 = 2 * m;
		aa = m * ( b - m ) * x / ( ( qam + m2 ) * ( a + m2 ) );
		d = 1.0 + aa * d;
		if ( fabs( d ) < FPMIN)
			d = FPMIN;

		c = 1.0 + aa / c;
		if ( fabs( c ) < FPMIN )
			c=FPMIN;

		d = 1.0 / d;
		h *= d * c;
		aa = -( a + m ) * ( qab + m ) * x / ( ( a + m2 ) * ( qap + m2 ) );
		d = 1.0 + aa * d;
		if ( fabs( d ) < FPMIN )
			d = FPMIN;

		c = 1.0 + aa / c;
		if ( fabs( c ) < FPMIN )
			c = FPMIN;

		d = 1.0 / d;
		del = d * c;
		h *= del;
		if ( fabs( del - 1.0) < BEPS )
			break;
	}

	if ( m > MAXIT )
		plog( "\nWarning: a or b too big (or MAXIT too small) in function: betacf");

	return h;
}


/***************************************************
BETACDF
Beta cumulative distribution function: incomplete beta function
Press et al. (1992) Numerical Recipes in C, 2nd Ed.
***************************************************/
double simulation::betacdf( double alpha, double beta, double x )
{
	double bt;

	if ( alpha <= 0.0 || beta <= 0.0 || x < 0.0 || x > 1.0 )
	{
		plog( "\nWarning: bad alpha, beta or x in function: betacdf" );
		return 0.0;
	}

	if ( x == 0.0 || x == 1.0 )
		bt = 0.0;
	else
		bt = exp( lgamma( alpha + beta ) - lgamma( alpha ) - lgamma( beta )
				 + alpha * log( x ) + beta * log( 1.0 - x ) );

	if ( x < ( alpha + 1.0 ) / ( alpha + beta + 2.0 ) )
		return bt * betacf( alpha, beta, x ) / alpha;
	else
		return 1.0 - bt * betacf( beta, alpha, 1.0 - x ) / beta;
}


/****************************************************
INIT_RANDOM
Set seed to all random generators
Pseudo-random number generator to extract draws
ran_gen_id = 0 : system (not pseudo) random device in (0,1)
ran_gen_id = 1 : Linear congruential in (0,1)
ran_gen_id = 2 : Mersenne-Twister in (0,1)
ran_gen_id = 3 : Linear congruential in [0,1)
ran_gen_id = 4 : Mersenne-Twister in [0,1)
ran_gen_id = 5 : Mersenne-Twister with 64 bits resolution in [0,1)
ran_gen_id = 6 : Lagged fibonacci with 24 bits resolution in [0,1)
ran_gen_id = 7 : Lagged fibonacci with 48 bits resolution in [0,1)
****************************************************/
void simulation::init_random( unsigned seed )
{
	idum = -seed;					// unused (legacy code only)
	lc1.seed( seed );				// linear congruential (internal)
	lc2.seed( seed );				// linear congruential (user)
	mt32.seed( seed );				// Mersenne-Twister 32 bits
	mt64.seed( seed );				// Mersenne-Twister 64 bits
	lf24.seed( seed );				// lagged fibonacci 24 bits
	lf48.seed( seed );				// lagged fibonacci 48 bits
}

template < class distr > double draw_rd( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_rd_lck );
#endif
	return d( sim->rd );
}

template < class distr > double draw_lc1( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_lc1_lck );
#endif
	return d( sim->lc1 );
}

template < class distr > double draw_lc2( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_lc2_lck );
#endif
	return d( sim->lc2 );
}

template < class distr > double draw_mt32( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_mt32_lck );
#endif
	return d( sim->mt32 );
}

template < class distr > double draw_mt64( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_mt64_lck );
#endif
	return d( sim->mt64 );
}

template < class distr > double draw_lf24( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_lf24_lck );
#endif
	return d( sim->lf24 );
}

template < class distr > double draw_lf48( simulation *sim, distr &d )
{
#ifndef _NP_
	// prevent concurrent draw by more than one thread
	lock_guard < mutex > lock( sim->draw_lf48_lck );
#endif
	return d( sim->lf48 );
}


/***************************************************
DRAW_GEN
Generate the draw using current generator object
***************************************************/
template < class distr > double simulation::draw_gen( distr &d )
{
	switch ( ran_gen_id )
	{
		case 0:						// system (not pseudo) random generator
			return draw_rd( this, d );
		case 1:						// linear congruential in (0,1)
		case 3:						// linear congruential in [0,1)
		default:
			return draw_lc2( this, d );

		case 2:						// Mersenne-Twister 32 bits in (0,1)
		case 4:						// Mersenne-Twister 32 bits in [0,1)
			return draw_mt32( this, d );

		case 5:						// Mersenne-Twister 64 bits in [0,1)
			return draw_mt64( this, d );

		case 6:						// lagged fibonacci 24 bits in [0,1)
			return draw_lf24( this, d );

		case 7:						// lagged fibonacci 48 bits in [0,1)
			return draw_lf48( this, d );
	}
}


/***************************************************
SET_RANDOM
Set the generator object to be used in draws
***************************************************/
void *simulation::set_random( int gen )
{
	if ( gen >= 0 && gen <= 7 )
	{
		ran_gen_id = gen;

		switch ( ran_gen_id )
		{
			case 0:						// system (not pseudo) random generator
				if ( ! HW_RAND_GEN )
					plog( "\nWarning: true random generator not available\n" );
				return ( ( void * ) & rd );

			case 1:						// linear congruential in (0,1)
			case 3:						// linear congruential in [0,1)
				return ( ( void * ) & lc2 );

			case 2:						// Mersenne-Twister 32 bits in (0,1)
			case 4:						// Mersenne-Twister 32 bits in [0,1)
				return ( ( void * ) & mt32 );

			case 5:						// Mersenne-Twister 64 bits in [0,1)
				return ( ( void * ) & mt64 );

			case 6:						// lagged fibonacci 24 bits in [0,1)
				return ( ( void * ) & lf24 );
				break;
			case 7:						// lagged fibonacci 48 bits in [0,1)
				return ( ( void * ) & lf48 );
		}
	}

	return NULL;
}


/****************************************************
RND_INT
****************************************************/
int simulation::rnd_int( int min, int max )
{
	uniform_int_distribution< int > distr( min, max );
	return draw_lc1( this, distr );
}


/***************************************************
RAN1
Call the preset pseudo-random number generator
Just generates numbers > 0 and < 1
***************************************************/
double simulation::ran1( long *unused )
{
	double ran;
	uniform_real_distribution< double > distr( 0, 1 );

	do
		ran = draw_gen( distr );
	while ( ran == 0.0 && ran_gen_id < 3 );

	return ran;
}


/****************************************************
UNIFORM
****************************************************/
double simulation::uniform( double min, double max )
{
	uniform_real_distribution< double > distr( min, max );
	return draw_gen( distr );
}


/****************************************************
UNIFORM_INT
****************************************************/
double simulation::uniform_int( double min, double max )
{
	uniform_int_distribution< int > distr( ( long ) min, ( long ) max );
	return draw_gen( distr );
}


/***************************************************
NORM
***************************************************/
double simulation::norm( double mean, double dev )
{
	static bool normStopErr;

	if ( dev < 0 )
	{
		warn_distr( normErrCnt, normStopErr, "norm", "negative standard deviation" );
		return mean;
	}

	normal_distribution< double > distr( mean, dev );
	return draw_gen( distr );
}


/***************************************************
LNORM
Return a draw from a lognormal distribution
***************************************************/
double simulation::lnorm( double mean, double dev )
{
	static bool lnormStopErr;

	if ( dev < 0 )
	{
		warn_distr( lnormErrCnt, lnormStopErr, "lnorm", "negative standard deviation" );
		return exp( mean );
	}

	lognormal_distribution< double > distr( mean, dev );
	return draw_gen( distr );
}


/****************************************************
GAMMA
****************************************************/
double simulation::gamma( double alpha, double beta )
{
	static bool gammaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		warn_distr( gammaErrCnt, gammaStopErr, "gamma", "non-positive alpha or beta parameter" );
		return 0.0;
	}

	gamma_distribution< double > distr( alpha, beta );
	return draw_gen( distr );
}


/****************************************************
BERNOULLI
****************************************************/
double simulation::bernoulli( double p )
{
	static bool bernoStopErr;

	if ( p < 0 || p > 1 )
	{
		warn_distr( bernoErrCnt, bernoStopErr, "bernoulli", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	bernoulli_distribution distr( p );
	return draw_gen( distr );
}


/****************************************************
POISSON
****************************************************/
double simulation::poisson( double mean )
{
	static bool poissStopErr;

	if ( mean < 0 )
	{
		warn_distr( poissErrCnt, poissStopErr, "poisson", "negative mean" );
		return 0.0;
	}

	poisson_distribution< int > distr( mean );
	return draw_gen( distr );
}


/****************************************************
GEOMETRIC
****************************************************/
double simulation::geometric( double p )
{
	static bool geomStopErr;

	if ( p < 0 || p > 1 )
	{
		warn_distr( geomErrCnt, geomStopErr, "geometric", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	geometric_distribution< int > distr( p );
	return draw_gen( distr );
}


/****************************************************
BINOMIAL
****************************************************/
double simulation::binomial( double p, double t )
{
	static bool binomStopErr;

	if ( p < 0 || p > 1 || t <= 0 )
	{
		warn_distr( binomErrCnt, binomStopErr, "binomial", "invalid parameter" );

		if ( p < 0 || t <= 0 )
			return 0.0;
		else
			return 1.0;
	}

	binomial_distribution< int > distr( t, p );
	return draw_gen( distr );
}


/***************************************************
CAUCHY
***************************************************/
double simulation::cauchy( double a, double b )
{
	static bool cauchStopErr;

	if ( b <= 0 )
	{
		warn_distr( cauchErrCnt, cauchStopErr, "cauchy", "non-positive b parameter" );
		return a;
	}

	cauchy_distribution< double > distr( a, b );
	return draw_gen( distr );
}


/***************************************************
CHI_SQUARED
***************************************************/
double simulation::chi_squared( double n )
{
	static bool chisqStopErr;

	if ( n <= 0 )
	{
		warn_distr( chisqErrCnt, chisqStopErr, "chi_squared", "non-positive n parameter" );
		return 0.0;
	}

	chi_squared_distribution< double > distr( n );
	return draw_gen( distr );
}


/***************************************************
EXPONENTIAL
***************************************************/
double simulation::exponential( double lambda )
{
	static bool expStopErr;

	if ( lambda <= 0 )
	{
		warn_distr( expErrCnt, expStopErr, "exponential", "non-positive lambda parameter" );
		return 0.0;
	}

	exponential_distribution< double > distr( lambda );
	return draw_gen( distr );
}


/***************************************************
FISHER
***************************************************/
double simulation::fisher( double m, double n )
{
	static bool fishStopErr;

	if ( m <= 0 || n <= 0 )
	{
		warn_distr( fishErrCnt, fishStopErr, "fisher", "invalid parameter" );
		return 0.0;
	}

	fisher_f_distribution< double > distr( m, n );
	return draw_gen( distr );
}


/***************************************************
STUDENT
***************************************************/
double simulation::student( double n )
{
	static bool studStopErr;

	if ( n <= 0 )
	{
		warn_distr( studErrCnt, studStopErr, "student", "non-positive n parameter" );
		return 0.0;
	}

	student_t_distribution< double > distr( n );
	return draw_gen( distr );
}


/***************************************************
WEIBULL
***************************************************/
double simulation::weibull( double a, double b )
{
	static bool weibStopErr;

	if ( a <= 0 || b <= 0 )
	{
		warn_distr( weibErrCnt, weibStopErr, "weibull", "non-positive a or b parameter" );
		return 0.0;
	}

	weibull_distribution< double > distr( a, b );
	return draw_gen( distr );
}


/***************************************************
BETA
Return a draw from a Beta(alfa,beta) distribution
***************************************************/
double simulation::beta( double alpha, double beta )
{
	static bool betaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		warn_distr( betaErrCnt, betaStopErr, "beta", "non-positive alpha or beta parameter" );

		if ( alpha < beta )
			return 0.0;
		else
			return 1.0;
	}

	gamma_distribution< double > distr1( alpha, 1.0 ), distr2( beta, 1.0 );
	double draw = draw_gen( distr1 );
	return draw / ( draw + draw_gen( distr2 ) );
}


/****************************************************
PARETO
****************************************************/
double simulation::pareto( double mu, double alpha )
{
	static bool paretStopErr;

	if ( mu <= 0 || alpha <= 0 )
	{
		warn_distr( paretErrCnt, paretStopErr, "pareto", "non-positive mu or alpha parameter" );
		return mu;
	}

	return mu / pow( 1 - ran1( ), 1 / alpha );
}


/****************************************************
BPARETO
****************************************************/
double simulation::bpareto( double alpha, double low, double high )
{
	static bool paretStopErr;

	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		warn_distr( paretErrCnt, paretStopErr, "bpareto", "non-positive alpha parameter or bounds or invalid bounds" );
		return max( low, 0 );
	}

	return pow( pow( low, alpha ) /
				( ran1( ) * ( pow( low / high, alpha ) - 1 ) + 1 ),
				1 / alpha );
}


/***************************************************
ALAPL
Return a draw from an asymmetric laplace distribution
***************************************************/
double simulation::alapl( double mu, double alpha1, double alpha2 )
{
	static bool alaplStopErr;

	if ( alpha1 <= 0 || alpha2 <= 0 )
	{
		warn_distr( alaplErrCnt, alaplStopErr, "alapl", "non-positive alpha1 or alpha2 parameter" );
		return mu;
	}

	double draw = ran1( );
	if ( draw < ( alpha1 / ( alpha1 + alpha2 ) ) )
		return mu + alpha1 * log( draw * ( 1 + alpha1 / alpha2 ) );
	else
		return mu - alpha2 * log( ( 1 - draw ) * ( 1 + alpha1 / alpha2 ) );
}


/****************************************************
WARN_DISTR
****************************************************/
#ifndef _NP_
void simulation::warn_distr( atomic < int > & errCnt, bool & stopErr, const char *distr, const char *msg )
#else
void simulation::warn_distr( int & errCnt, bool & stopErr, const char *distr, const char *msg )
#endif
{
	if ( ++errCnt < ERR_LIM )	// prevent slow down due to I/O
	{
		plog( "\nWarning: %s in function '%s'", msg, distr );
		stopErr = false;
	}
	else
		if ( ! stopErr )
		{
			plog( "\nWarning: too many warnings in function '%s', stop reporting...\n", distr );
			stopErr = true;
		}
}


/***************************************************
INIT_MATH_ERROR
Initialize the math functions error controls
***************************************************/
void simulation::init_math_error( void )
{
	normErrCnt = lnormErrCnt = gammaErrCnt = bernoErrCnt = poissErrCnt = 0;
	geomErrCnt = binomErrCnt = cauchErrCnt = chisqErrCnt = expErrCnt = 0;
	fishErrCnt = studErrCnt = weibErrCnt = betaErrCnt = paretErrCnt = alaplErrCnt = 0;
}
