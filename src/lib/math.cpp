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


/*************************************************************
 ROUND_DIGITS
 *************************************************************/
double lsd::equation::round_digits( double value, int digits )
{
	if ( value == 0.0 )
		return 0.0;

	double factor = pow( 10.0, digits - ceil( log10( fabs( value ) ) ) );

	return round( value * factor ) / factor;
}


/*************************************************************
 IPOW
 Integer exponentiation
 *************************************************************/
double lsd::equation::ipow( double base, double exp )
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


/*************************************************************
 FACT
 Factorial function
 *************************************************************/
double lsd::equation::fact( double x )
{
	x = floor( x );
	if ( x < 0.0 )
	{
		_sim_->plog( "\nWarning: bad x in function: fact" );
		return 0.0;
	}

	double fact = 1.0;
	long i = 1;
	while (i <= x)
		fact *= i++;

	return fact;
}


/*************************************************************
 MEDIAN
 *************************************************************/
double lsd::simulation::median( d_vecT & v )
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


/*************************************************************
 T_STAR
 Student t distribution  statistic for given
 degrees of freedom and confidence level (in %)
 *************************************************************/
double lsd::equation::t_star( int df, double cl )
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


/*************************************************************
 Z_STAR
 Standard normal distribution statistic for given
 confidence level (in %)
 *************************************************************/
double lsd::equation::z_star( double cl )
{
	int i;

	for ( i = 0; i < Z_CLEVS - 1; ++i )
		if ( cl <= 100 * z_dist_cl[ i ] )
			break;

	return z_dist_st[ i ];
}


/*************************************************************
 UNIFCDF
 Uniform cumulative distribution function
 *************************************************************/
double lsd::equation::unifcdf( double a, double b, double x )
{
	if ( a >= b )
	{
		_sim_->plog( "\nWarning: bad a or b in function: uniformcdf" );
		return 0.0;
	}

	if ( x <= a )
		return 0.0;
	if ( x >= b )
		return 1.0;

	return ( x - a ) / ( b - a );
}


/*************************************************************
 POISSONCDF
 Poisson cumulative distribution function
 *************************************************************/
double lsd::equation::poissoncdf( double lambda, double k )
{
	k = floor( k );
	if ( lambda <= 0.0 || k < 0.0 )
	{
		_sim_->plog( "\nWarning: bad lambda or k in function: poissoncdf" );
		return 0.0;
	}

	double sum = 0.0;
	long i;
	for ( i = 0; i <= k; i++ )
		sum += pow( lambda, i ) / fact( i );

	return exp( -lambda ) * sum;
}


/*************************************************************
 PARETOCDF
 Pareto cumulative distribution function
 *************************************************************/
double lsd::equation::paretocdf( double mu, double alpha, double x )
{
	if ( mu <= 0 || alpha <= 0 )
	{
		_sim_->plog( "\nWarning: bad mu, alpha in function: paretocdf" );
		return 0.0;
	}

	if ( x < mu )
		return 0.0;
	else
		return 1.0 - pow( mu / x, alpha );
}


/*************************************************************
 BPARETOCDF
 Bounded Pareto cumulative distribution function
 *************************************************************/
double lsd::equation::bparetocdf( double alpha, double low, double high, double x )
{
	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		_sim_->plog( "\nWarning: bad alpha, low or high in function: bparetocdf" );
		return 0.0;
	}

	if ( x < low )
		return 0.0;
	else
		return ( 1 - pow( low, alpha ) * pow( x, - alpha ) ) /
			   ( 1 - pow( low / high, alpha ) ) ;
}


/*************************************************************
 NORMCDF
 Normal cumulative distribution function
 *************************************************************/
double lsd::equation::normcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 )
	{
		_sim_->plog( "\nWarning: bad sigma in function: normalcdf" );
		return 0.0;
	}

	return 0.5 * ( 1 + erf( ( x - mu ) / ( sigma * sqrt( 2.0 ) ) ) );
}


/*************************************************************
 LNORMCDF
 Lognormal cumulative distribution function
 *************************************************************/
double lsd::equation::lnormcdf( double mu, double sigma, double x )
{
	if ( sigma <= 0.0 || x <= 0.0 )
	{
		_sim_->plog( "\nWarning: bad sigma or x in function: lnormalcdf" );
		return 0.0;
	}

	return 0.5 + 0.5 * erf( ( log( x ) - mu ) / ( sigma * sqrt( 2.0 ) ) );
}


/*************************************************************
 ALAPLCDF
 Asymmetric laplace cumulative distribution
 function
 *************************************************************/
double lsd::equation::alaplcdf( double mu, double alpha1, double alpha2, double x )
{
	if ( alpha1 <= 0.0 || alpha2 <= 0.0 )
	{
		_sim_->plog( "\nWarning: bad alpha in function: alaplcdf" );
		return 0.0;
	}

	if ( x < mu )									// cdf up to upper bound
		return 0.5 * exp( ( x - mu ) / alpha1 );
	else
		return 1 - 0.5 * exp( -( x - mu ) / alpha2 );
}


/*************************************************************
 BETACF
 Beta distribution: continued fraction evaluation
 function
 Press et al. (1992) Numerical Recipes in C, 2nd Ed.
 *************************************************************/
#define MAXIT 100
#define BEPS 3.0e-7
#define FPMIN 1.0e-30

double lsd::simulation::betacf( double a, double b, double x )
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


/*************************************************************
 BETACDF
 Beta cumulative distribution function: incomplete
 beta function
 Press et al. (1992) Numerical Recipes in C, 2nd Ed.
 *************************************************************/
double lsd::equation::betacdf( double alpha, double beta, double x )
{
	double bt;

	if ( alpha <= 0.0 || beta <= 0.0 || x < 0.0 || x > 1.0 )
	{
		_sim_->plog( "\nWarning: bad alpha, beta or x in function: betacdf" );
		return 0.0;
	}

	if ( x == 0.0 || x == 1.0 )
		bt = 0.0;
	else
		bt = exp( lgamma( alpha + beta ) - lgamma( alpha ) - lgamma( beta )
				 + alpha * log( x ) + beta * log( 1.0 - x ) );

	if ( x < ( alpha + 1.0 ) / ( alpha + beta + 2.0 ) )
		return bt * _sim_->betacf( alpha, beta, x ) / alpha;
	else
		return 1.0 - bt * _sim_->betacf( beta, alpha, 1.0 - x ) / beta;
}


/*************************************************************
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
 *************************************************************/
void lsd::simulation::init_random( unsigned seed )
{
	idum = -seed;					// unused (legacy code only)
	lc1.seed( seed );				// linear congruential (internal)
	lc2.seed( seed );				// linear congruential (user)
	mt32.seed( seed );				// Mersenne-Twister 32 bits
	mt64.seed( seed );				// Mersenne-Twister 64 bits
	lf24.seed( seed );				// lagged fibonacci 24 bits
	lf48.seed( seed );				// lagged fibonacci 48 bits
}

template < class distr > double lsd::simulation::draw_rd( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_rd_lck );

	return d( rd );
}

template < class distr > double lsd::simulation::draw_lc1( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_lc1_lck );

	return d( lc1 );
}

template < class distr > double lsd::simulation::draw_lc2( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_lc2_lck );

	return d( lc2 );
}

template < class distr > double lsd::simulation::draw_mt32( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_mt32_lck );

	return d( mt32 );
}

template < class distr > double lsd::simulation::draw_mt64( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_mt64_lck );

	return d( mt64 );
}

template < class distr > double lsd::simulation::draw_lf24( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_lf24_lck );

	return d( lf24 );
}

template < class distr > double lsd::simulation::draw_lf48( distr &d )
{
	// prevent concurrent draw by more than one thread
	l_guardT lock( draw_lf48_lck );

	return d( lf48 );
}


/*************************************************************
 DRAW_GEN
 Generate the draw using current generator object
 *************************************************************/
template < class distr > double lsd::simulation::draw_gen( distr &d )
{
	switch ( ran_gen_id )
	{
		case 0:						// system (not pseudo) random generator
			return draw_rd( d );
		case 1:						// linear congruential in (0,1)
		case 3:						// linear congruential in [0,1)
		default:
			return draw_lc2( d );

		case 2:						// Mersenne-Twister 32 bits in (0,1)
		case 4:						// Mersenne-Twister 32 bits in [0,1)
			return draw_mt32( d );

		case 5:						// Mersenne-Twister 64 bits in [0,1)
			return draw_mt64( d );

		case 6:						// lagged fibonacci 24 bits in [0,1)
			return draw_lf24( d );

		case 7:						// lagged fibonacci 48 bits in [0,1)
			return draw_lf48( d );
	}
}


/*************************************************************
 SET_RANDOM
 Set the generator object to be used in draws
 *************************************************************/
void *lsd::simulation::set_random( int gen )
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


/*************************************************************
 RND_INT
 *************************************************************/
int lsd::simulation::rnd_int( int min, int max )
{
	std::uniform_int_distribution < int > distr( min, max );
	return draw_lc1( distr );
}


/*************************************************************
 RAN1
 Call the preset pseudo-random number generator
 Just generates numbers > 0 and < 1
 *************************************************************/
double lsd::equation::_ran1_( long *unused )
{
	double ran;
	std::uniform_real_distribution < double > distr( 0, 1 );

	do
		ran = _sim_->draw_gen( distr );
	while ( ran == 0.0 && _sim_->ran_gen_id < 3 );

	return ran;
}


/*************************************************************
 UNIFORM
 *************************************************************/
double lsd::equation::uniform( double min, double max )
{
	std::uniform_real_distribution < double > distr( min, max );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 UNIFORM_INT
 *************************************************************/
double lsd::equation::uniform_int( double min, double max )
{
	std::uniform_int_distribution < int > distr( ( long ) min, ( long ) max );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 NORM
 *************************************************************/
double lsd::equation::norm( double mean, double dev )
{
	static bool normStopErr;

	if ( dev < 0 )
	{
		_sim_->warn_distr( _sim_->normErrCnt, normStopErr, "norm", "negative standard deviation" );
		return mean;
	}

	std::normal_distribution < double > distr( mean, dev );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 LNORM
 Return a draw from a lognormal distribution
 *************************************************************/
double lsd::equation::lnorm( double mean, double dev )
{
	static bool lnormStopErr;

	if ( dev < 0 )
	{
		_sim_->warn_distr( _sim_->lnormErrCnt, lnormStopErr, "lnorm", "negative standard deviation" );
		return exp( mean );
	}

	std::lognormal_distribution < double > distr( mean, dev );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 GAMMA
 *************************************************************/
double lsd::equation::gamma( double alpha, double beta )
{
	static bool gammaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		_sim_->warn_distr( _sim_->gammaErrCnt, gammaStopErr, "gamma", "non-positive alpha or beta parameter" );
		return 0.0;
	}

	std::gamma_distribution < double > distr( alpha, beta );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 BERNOULLI
 *************************************************************/
double lsd::equation::bernoulli( double p )
{
	static bool bernoStopErr;

	if ( p < 0 || p > 1 )
	{
		_sim_->warn_distr( _sim_->bernoErrCnt, bernoStopErr, "bernoulli", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	std::bernoulli_distribution distr( p );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 POISSON
 *************************************************************/
double lsd::equation::poisson( double mean )
{
	static bool poissStopErr;

	if ( mean < 0 )
	{
		_sim_->warn_distr( _sim_->poissErrCnt, poissStopErr, "poisson", "negative mean" );
		return 0.0;
	}

	std::poisson_distribution < int > distr( mean );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 GEOMETRIC
 *************************************************************/
double lsd::equation::geometric( double p )
{
	static bool geomStopErr;

	if ( p < 0 || p > 1 )
	{
		_sim_->warn_distr( _sim_->geomErrCnt, geomStopErr, "geometric", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	std::geometric_distribution < int > distr( p );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 BINOMIAL
 *************************************************************/
double lsd::equation::binomial( double p, double t )
{
	static bool binomStopErr;

	if ( p < 0 || p > 1 || t <= 0 )
	{
		_sim_->warn_distr( _sim_->binomErrCnt, binomStopErr, "binomial", "invalid parameter" );

		if ( p < 0 || t <= 0 )
			return 0.0;
		else
			return 1.0;
	}

	std::binomial_distribution < int > distr( t, p );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 CAUCHY
 *************************************************************/
double lsd::equation::cauchy( double a, double b )
{
	static bool cauchStopErr;

	if ( b <= 0 )
	{
		_sim_->warn_distr( _sim_->cauchErrCnt, cauchStopErr, "cauchy", "non-positive b parameter" );
		return a;
	}

	std::cauchy_distribution < double > distr( a, b );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 CHI_SQUARED
 *************************************************************/
double lsd::equation::chi_squared( double n )
{
	static bool chisqStopErr;

	if ( n <= 0 )
	{
		_sim_->warn_distr( _sim_->chisqErrCnt, chisqStopErr, "chi_squared", "non-positive n parameter" );
		return 0.0;
	}

	std::chi_squared_distribution < double > distr( n );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 EXPONENTIAL
 *************************************************************/
double lsd::equation::exponential( double lambda )
{
	static bool expStopErr;

	if ( lambda <= 0 )
	{
		_sim_->warn_distr( _sim_->expErrCnt, expStopErr, "exponential", "non-positive lambda parameter" );
		return 0.0;
	}

	std::exponential_distribution < double > distr( lambda );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 FISHER
 *************************************************************/
double lsd::equation::fisher( double m, double n )
{
	static bool fishStopErr;

	if ( m <= 0 || n <= 0 )
	{
		_sim_->warn_distr( _sim_->fishErrCnt, fishStopErr, "fisher", "invalid parameter" );
		return 0.0;
	}

	std::fisher_f_distribution < double > distr( m, n );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 STUDENT
 *************************************************************/
double lsd::equation::student( double n )
{
	static bool studStopErr;

	if ( n <= 0 )
	{
		_sim_->warn_distr( _sim_->studErrCnt, studStopErr, "student", "non-positive n parameter" );
		return 0.0;
	}

	std::student_t_distribution < double > distr( n );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 WEIBULL
 *************************************************************/
double lsd::equation::weibull( double a, double b )
{
	static bool weibStopErr;

	if ( a <= 0 || b <= 0 )
	{
		_sim_->warn_distr( _sim_->weibErrCnt, weibStopErr, "weibull", "non-positive a or b parameter" );
		return 0.0;
	}

	std::weibull_distribution < double > distr( a, b );
	return _sim_->draw_gen( distr );
}


/*************************************************************
 BETA
 Return a draw from a Beta(alfa,beta) distribution
 *************************************************************/
double lsd::equation::beta( double alpha, double beta )
{
	static bool betaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		_sim_->warn_distr( _sim_->betaErrCnt, betaStopErr, "beta", "non-positive alpha or beta parameter" );

		if ( alpha < beta )
			return 0.0;
		else
			return 1.0;
	}

	std::gamma_distribution < double > distr1( alpha, 1.0 ), distr2( beta, 1.0 );
	double draw = _sim_->draw_gen( distr1 );
	return draw / ( draw + _sim_->draw_gen( distr2 ) );
}


/*************************************************************
 PARETO
 *************************************************************/
double lsd::equation::pareto( double mu, double alpha )
{
	static bool paretStopErr;

	if ( mu <= 0 || alpha <= 0 )
	{
		_sim_->warn_distr( _sim_->paretErrCnt, paretStopErr, "pareto", "non-positive mu or alpha parameter" );
		return mu;
	}

	return mu / pow( 1 - _ran1_( ), 1 / alpha );
}


/*************************************************************
 BPARETO
 *************************************************************/
double lsd::equation::bpareto( double alpha, double low, double high )
{
	static bool paretStopErr;

	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		_sim_->warn_distr( _sim_->paretErrCnt, paretStopErr, "bpareto", "non-positive alpha parameter or bounds or invalid bounds" );
		return std::max( low, 0. );
	}

	return pow( pow( low, alpha ) /
				( _ran1_( ) * ( pow( low / high, alpha ) - 1 ) + 1 ),
				1 / alpha );
}


/*************************************************************
 ALAPL
 Return a draw from an asymmetric laplace distribution
 *************************************************************/
double lsd::equation::alapl( double mu, double alpha1, double alpha2 )
{
	static bool alaplStopErr;

	if ( alpha1 <= 0 || alpha2 <= 0 )
	{
		_sim_->warn_distr( _sim_->alaplErrCnt, alaplStopErr, "alapl", "non-positive alpha1 or alpha2 parameter" );
		return mu;
	}

	double draw = _ran1_( );
	if ( draw < ( alpha1 / ( alpha1 + alpha2 ) ) )
		return mu + alpha1 * log( draw * ( 1 + alpha1 / alpha2 ) );
	else
		return mu - alpha2 * log( ( 1 - draw ) * ( 1 + alpha1 / alpha2 ) );
}


/*************************************************************
 WARN_DISTR
 *************************************************************/
void lsd::simulation::warn_distr( std::atomic < int > & errCnt, bool & stopErr, const char *distr, const char *msg )
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


/*************************************************************
 INIT_MATH_ERROR
 Initialize the math functions error controls
 *************************************************************/
void lsd::simulation::init_math_error( void )
{
	normErrCnt = lnormErrCnt = gammaErrCnt = bernoErrCnt = poissErrCnt = 0;
	geomErrCnt = binomErrCnt = cauchErrCnt = chisqErrCnt = expErrCnt = 0;
	fishErrCnt = studErrCnt = weibErrCnt = betaErrCnt = paretErrCnt = alaplErrCnt = 0;
}
