/*************************************************************

	LSD 9.0 - January 2026
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
 ROUND_DIGITS (*)
 *************************************************************/
double lsd::equation::round_digits( double value, int digits )
{
	if ( value == 0.0 )
		return 0.0;

	double factor = pow( 10.0, digits - ceil( log10( fabs( value ) ) ) );

	return std::round( value * factor ) / factor;
}


/*************************************************************
 IPOW (*)
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
 FACT (*)
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
 MEAN (*)
 *************************************************************/
double lsd::equation::mean( d_vecT & v )
{
	if ( v.empty( ) )
		return NAN;

	return std::accumulate( v.begin( ), v.end( ), 0. ) / v.size( );
}


/*************************************************************
 MED (*)
 Preserves the vector order
 *************************************************************/
double lsd::equation::med( d_vecT v )
{
	return median( v );
}


/*************************************************************
 MEDIAN (*)
 It changes the original vector order!
 *************************************************************/
double lsd::median( d_vecT & v )
{
	if ( v.empty( ) )
		return NAN;

	int s = v.size( );
	int n = s / 2;
	auto p = v.begin( ) + n;
	std::nth_element( v.begin( ), p, v.end( ) );

	if ( s % 2 != 0 )
		return v[ n ];
	else
		return ( * std::max_element( v.begin( ), p ) + v[ n ] ) / 2.;
}


/*************************************************************
 SD (*)
 *************************************************************/
double lsd::equation::sd( d_vecT & v )
{
	if ( v.empty( ) )
		return NAN;

	d_vecT d( v.size( ) );
	double m = mean( v );
	std::transform( v.begin( ), v.end( ), d.begin( ), [ m ]( double x ) { return x - m; } );

	return std::sqrt( std::inner_product( d.begin( ), d.end( ), d.begin( ), 0. ) / v.size( ) );
}


/*************************************************************
 MAD (*)
 *************************************************************/
double lsd::equation::mad( d_vecT & v )
{
	if ( v.empty( ) )
		return NAN;

	d_vecT d( v.size( ) );
	double m = median( v );
	std::transform( v.begin( ), v.end( ), d.begin( ), [ m ]( double x ) { return fabs( x - m ); } );

	return median( d );
}


/*************************************************************
 COV (*)
 *************************************************************/
double lsd::equation::cov( d_vecT & u, d_vecT & v )
{
	if ( u.empty( ) || u.size( ) != v.size( ) )
		return NAN;

	d_vecT d( u.size( ) );
	double mu = mean( u );
	double mv = mean( v );
	std::transform( u.begin( ), u.end( ), v.begin( ), d.begin( ), [ mu, mv ]( double x, double y ) { return ( x - mu ) * ( y - mv ); } );

	return mean( d );
}


/*************************************************************
 COM (*)
 *************************************************************/
double lsd::equation::com( d_vecT & u, d_vecT & v )
{
	if ( u.empty( ) || u.size( ) != v.size( ) )
		return NAN;

	d_vecT d( u.size( ) );
	double mu = median( u );
	double mv = median( v );
	std::transform( u.begin( ), u.end( ), v.begin( ), d.begin( ), [ mu, mv ]( double x, double y ) { return ( x - mu ) * ( y - mv ); } );

	return median( d );
}


/*************************************************************
 T_STAR (*)
 Student t distribution  statistic for given
 degrees of freedom and confidence level (alpha/2) (in %)
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
 Z_STAR (*)
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
 UNIFCDF (*)
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
 POISSONCDF (*)
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
 PARETOCDF (*)
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
 BPARETOCDF (*)
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
 NORMCDF (*)
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
 LNORMCDF (*)
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
 ALAPLCDF (*)
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
 BETACF (*)
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
 BETACDF (*)
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
 ALLOC_PRNG
 Allocate the pseudo-random number generator of object
 *************************************************************/
void lsd::object::alloc_prng( void )
{
	switch ( prng_type )
	{
		case 0:						// system (not pseudo) random device in (0,1)
			if ( HW_RAND_GEN )
			{
				prng = ( void * ) new std::random_device;
				break;
			}
			else
			{
				attr->cont->sim->plog( "\nWarning: true random generator not available\n" );
				prng_type = 1;
			}
		case 1:						// Linear congruential in (0,1)
		case 3:						// linear congruential in [0,1)
			prng = ( void * ) new std::minstd_rand;
			break;
		case 2:						// Mersenne-Twister in (0,1)
		case 4:						// Mersenne-Twister in [0,1)
			prng = ( void * ) new std::mt19937;
			break;
		case 5:						// Mersenne-Twister 64 bits resolution in [0,1)
			prng = ( void * ) new std::mt19937_64;
			break;
		case 6:						// Lagged fibonacci 24 bits resolution in [0,1)
			prng = ( void * ) new std::ranlux24;
			break;
		case 7:						// Lagged fibonacci 48 bits resolution in [0,1)
			prng = ( void * ) new std::ranlux48;
			break;
	}
}


/*************************************************************
 FREE_PRNG
 *************************************************************/
void lsd::object::free_prng( void )
{
	switch ( prng_type )
	{
		case 0:
			delete ( std::random_device * ) prng;
			break;
		case 1:
		case 3:
			delete ( std::minstd_rand * ) prng;
			break;
		case 2:
		case 4:
			delete ( std::mt19937 * ) prng;
			break;
		case 5:
			delete ( std::mt19937_64 * ) prng;
			break;
		case 6:
			delete ( std::ranlux24 * ) prng;
			break;
		case 7:
			delete ( std::ranlux48 * ) prng;
			break;
	}

	prng = NULL;
}


/*************************************************************
 SET_RND_GEN (*)
 *************************************************************/
void *lsd::object::set_rnd_gen( int type )
{
	// change object generator
	if ( type != prng_type && type >= 0 && type <= 7 )
	{
		free_prng( );
		prng_type = type;
		alloc_prng( );
		set_rnd_seed( prng_seed == 0 ? up->attr->cont->sim->seeder( ) : prng_seed );
	}
	else
		if ( type < 0 )
		{
			free_prng( );
			prng_type = -1;

			if ( up != NULL )
				prng = up->prng;		// parent's PRNG
		}
		else
			return prng;

	// update descendants
	for ( auto cb = b; cb != NULL; cb = cb->next )
		for ( auto cur = cb->head; cur != NULL; cur = cur->next )
			if ( cur->prng_type < 0 )
				cur->set_rnd_gen( -1 );	// update only dependent descendants

	return prng;
}


/*************************************************************
 SET_RND_SEED (*)
 *************************************************************/
double lsd::object::set_rnd_seed( u_long32T seed )
{
	prng_seed = seed;

	if ( prng != NULL )
		switch ( prng_type )
		{
			case 1:
			case 3:
				( ( std::minstd_rand * ) prng )->seed( seed );
				break;
			case 2:
			case 4:
				( ( std::mt19937 * ) prng )->seed( seed );
				break;
			case 5:
				( ( std::mt19937_64 * ) prng )->seed( seed );
				break;
			case 6:
				( ( std::ranlux24 * ) prng )->seed( seed );
				break;
			case 7:
				( ( std::ranlux48 * ) prng )->seed( seed );
				break;
		}

	return prng_seed;
}


/*************************************************************
 GET_RND_SEED (*)
 *************************************************************/
double lsd::object::get_rnd_seed( void )
{
	return prng_seed;
}


/*************************************************************
 SEEDER
 *************************************************************/
u_long32T lsd::simulation::seeder( u_long32T seed )
{
	static std::mt19937 rd;
	static std::vector< u_long32T > seeds;

	if ( seed > 0 || seeds.size( ) == 0 )
	{
		std::uniform_int_distribution< int > u( 0, 9 );

		if ( seed > 0 )
			rd.seed( seed );

		seeds.resize( 1000 );
		std::seed_seq seq { u( rd ), u( rd ), u( rd ), u( rd ) };
		seq.generate( seeds.begin( ), seeds.end( ) );
	}

	if ( seed == 0 )
	{
		seed = seeds.back( );
		seeds.pop_back( );

		return seed;
	}

	return 0;
}


/*************************************************************
 DRAW_PRNG(_X(_Y))
 Generate the draw using current generator object
 *************************************************************/
template < class dT > double lsd::object::draw_prng( dT & d )
{
	l_guardT lock( obj_draw_lck );

	switch ( prng_type )
	{
		case 0:						// system (not pseudo) random generator
			return draw_prng_0( d );
		case 1:						// linear congruential in (0,1)
		case 3:						// linear congruential in [0,1)
			return draw_prng_1_3( d );

		case 2:						// Mersenne-Twister 32 bits in (0,1)
		case 4:						// Mersenne-Twister 32 bits in [0,1)
			return draw_prng_2_4( d );

		case 5:						// Mersenne-Twister 64 bits in [0,1)
			return draw_prng_5( d );

		case 6:						// lagged fibonacci 24 bits in [0,1)
			return draw_prng_6( d );

		case 7:						// lagged fibonacci 48 bits in [0,1)
			return draw_prng_7( d );

		default:
			if ( up != NULL )
				return up->draw_prng( d );
	}

	return NAN;
}

template < class dT > double lsd::object::draw_prng_0( dT & d )
{
	return d( *( ( std::random_device * ) prng ) );
}

template < class dT > double lsd::object::draw_prng_1_3( dT & d )
{
	return d( *( ( std::minstd_rand * ) prng ) );
}

template < class dT > double lsd::object::draw_prng_2_4( dT & d )
{
	return d( *( ( std::mt19937 * ) prng ) );
}

template < class dT > double lsd::object::draw_prng_5( dT & d )
{
	return d( *( ( std::mt19937_64 * ) prng ) );
}

template < class dT > double lsd::object::draw_prng_6( dT & d )
{
	return d( *( ( std::ranlux24 * ) prng ) );
}

template < class dT > double lsd::object::draw_prng_7( dT & d )
{
	return d( *( ( std::ranlux48 * ) prng ) );
}


/*************************************************************
 RND_01 (*)
 Generates numbers >(=) 0 and < 1
 *************************************************************/
double lsd::object::rnd_01( long *unused )
{
	double rnd;
	std::uniform_real_distribution < double > d( 0, 1 );

	do
		rnd = draw_prng( d );
	while ( rnd == 0.0 && ( prng_type == 1 || prng_type == 2 ) );

	return rnd;
}


/*************************************************************
 RND_UNIFORM (*)
 *************************************************************/
double lsd::object::rnd_uniform( double min, double max )
{
	std::uniform_real_distribution < double > d( min, max );
	return draw_prng( d );
}


/*************************************************************
 RND_UNIFORM_INT (*)
 *************************************************************/
double lsd::object::rnd_uniform_int( double min, double max )
{
	std::uniform_int_distribution < int > d( ( long ) min, ( long ) max );
	return draw_prng( d );
}


/*************************************************************
 RND_NORM (*)
 *************************************************************/
double lsd::object::rnd_normal( double mean, double dev )
{
	static bool normStopErr;

	if ( dev < 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->normErrCnt, normStopErr, "norm", "negative standard deviation" );
		return mean;
	}

	std::normal_distribution < double > d( mean, dev );
	return draw_prng( d );
}


/*************************************************************
 RND_LOG_NORMAL (*)
 Return a draw from a lognormal distribution
 *************************************************************/
double lsd::object::rnd_log_normal( double mean, double dev )
{
	static bool lnormStopErr;

	if ( dev < 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->lnormErrCnt, lnormStopErr, "lnorm", "negative standard deviation" );
		return exp( mean );
	}

	std::lognormal_distribution < double > d( mean, dev );
	return draw_prng( d );
}


/*************************************************************
 RND_GAMMA (*)
 *************************************************************/
double lsd::object::rnd_gamma( double alpha, double beta )
{
	static bool gammaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->gammaErrCnt, gammaStopErr, "gamma", "non-positive alpha or beta parameter" );
		return 0.0;
	}

	std::gamma_distribution < double > d( alpha, beta );
	return draw_prng( d );
}


/*************************************************************
 RND_BERNOULLI (*)
 *************************************************************/
double lsd::object::rnd_bernoulli( double p )
{
	static bool bernoStopErr;

	if ( p < 0 || p > 1 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->bernoErrCnt, bernoStopErr, "bernoulli", "probability out of \\[0, 1\\]" );

		if ( p < 0 )
			return 0.0;
		else
			return 1.0;
	}

	std::bernoulli_distribution d( p );
	return draw_prng( d );
}


/*************************************************************
 RND_POISSON (*)
 *************************************************************/
double lsd::object::rnd_poisson( double mean )
{
	static bool poissStopErr;

	if ( mean < 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->poissErrCnt, poissStopErr, "poisson", "negative mean" );
		return 0.0;
	}

	std::poisson_distribution < int > d( mean );
	return draw_prng( d );
}


/*************************************************************
 RND_GEOMETRIC (*)
 *************************************************************/
double lsd::object::rnd_geometric( double p )
{
	static bool geomStopErr;

	if ( p <= 0 || p >= 1 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->geomErrCnt, geomStopErr, "geometric", "probability out of \\[0, 1\\]" );

		return 0.0;
	}

	std::geometric_distribution < int > d( p );
	return draw_prng( d );
}


/*************************************************************
 RND_BINOMIAL (*)
 *************************************************************/
double lsd::object::rnd_binomial( double p, double t )
{
	static bool binomStopErr;

	if ( p < 0 || p > 1 || t <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->binomErrCnt, binomStopErr, "binomial", "invalid parameter" );

		if ( p < 0 || t <= 0 )
			return 0.0;
		else
			return 1.0;
	}

	std::binomial_distribution < int > d( t, p );
	return draw_prng( d );
}


/*************************************************************
 RND_CAUCHY (*)
 *************************************************************/
double lsd::object::rnd_cauchy( double a, double b )
{
	static bool cauchStopErr;

	if ( b <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->cauchErrCnt, cauchStopErr, "cauchy", "non-positive b parameter" );
		return a;
	}

	std::cauchy_distribution < double > d( a, b );
	return draw_prng( d );
}


/*************************************************************
 RND_CHI_SQUARED (*)
 *************************************************************/
double lsd::object::rnd_chi_squared( double n )
{
	static bool chisqStopErr;

	if ( n <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->chisqErrCnt, chisqStopErr, "chi_squared", "non-positive n parameter" );
		return 0.0;
	}

	std::chi_squared_distribution < double > d( n );
	return draw_prng( d );
}


/*************************************************************
 RND_EXPONENTIAL (*)
 *************************************************************/
double lsd::object::rnd_exponential( double lambda )
{
	static bool expStopErr;

	if ( lambda <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->expErrCnt, expStopErr, "exponential", "non-positive lambda parameter" );
		return 0.0;
	}

	std::exponential_distribution < double > d( lambda );
	return draw_prng( d );
}


/*************************************************************
 RND_FISHER (*)
 *************************************************************/
double lsd::object::rnd_fisher( double m, double n )
{
	static bool fishStopErr;

	if ( m <= 0 || n <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->fishErrCnt, fishStopErr, "fisher", "invalid parameter" );
		return 0.0;
	}

	std::fisher_f_distribution < double > d( m, n );
	return draw_prng( d );
}


/*************************************************************
 RND_STUDENT (*)
 *************************************************************/
double lsd::object::rnd_student( double n )
{
	static bool studStopErr;

	if ( n <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->studErrCnt, studStopErr, "student", "non-positive n parameter" );
		return 0.0;
	}

	std::student_t_distribution < double > d( n );
	return draw_prng( d );
}


/*************************************************************
 RND_WEIBULL (*)
 *************************************************************/
double lsd::object::rnd_weibull( double a, double b )
{
	static bool weibStopErr;

	if ( a <= 0 || b <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->weibErrCnt, weibStopErr, "weibull", "non-positive a or b parameter" );
		return 0.0;
	}

	std::weibull_distribution < double > d( a, b );
	return draw_prng( d );
}


/*************************************************************
 RND_BETA (*)
 Return a draw from a Beta(alfa,beta) distribution
 *************************************************************/
double lsd::object::rnd_beta( double alpha, double beta )
{
	static bool betaStopErr;

	if ( alpha <= 0 || beta <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->betaErrCnt, betaStopErr, "beta", "non-positive alpha or beta parameter" );

		if ( alpha < beta )
			return 0.0;
		else
			return 1.0;
	}

	std::gamma_distribution < double > d1( alpha, 1.0 ), d2( beta, 1.0 );
	double draw = draw_prng( d1 );
	return draw / ( draw + draw_prng( d2 ) );
}


/*************************************************************
 RND_PARETO (*)
 *************************************************************/
double lsd::object::rnd_pareto( double mu, double alpha )
{
	static bool paretStopErr;

	if ( mu <= 0 || alpha <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->paretErrCnt, paretStopErr, "pareto", "non-positive mu or alpha parameter" );
		return mu;
	}

	return mu / pow( 1 - rnd_01( ), 1 / alpha );
}


/*************************************************************
 RND_BPARETO (*)
 *************************************************************/
double lsd::object::rnd_bpareto( double alpha, double low, double high )
{
	static bool paretStopErr;

	if ( alpha <= 0 || low <= 0 || low >= high )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->paretErrCnt, paretStopErr, "bpareto", "non-positive alpha parameter or bounds or invalid bounds" );
		return std::max( low, 0. );
	}

	return pow( pow( low, alpha ) /
				( rnd_01( ) * ( pow( low / high, alpha ) - 1 ) + 1 ),
				1 / alpha );
}


/*************************************************************
 RND_ALAPLACE (*)
 Return a draw from an asymmetric laplace distribution
 *************************************************************/
double lsd::object::rnd_alaplace( double mu, double alpha1, double alpha2 )
{
	static bool alaplStopErr;

	if ( alpha1 <= 0 || alpha2 <= 0 )
	{
		up->attr->cont->sim->warn_distr( up->attr->cont->sim->alaplErrCnt, alaplStopErr, "alapl", "non-positive alpha1 or alpha2 parameter" );
		return mu;
	}

	double draw = rnd_01( );
	if ( draw < ( alpha1 / ( alpha1 + alpha2 ) ) )
		return mu + alpha1 * log( draw * ( 1 + alpha1 / alpha2 ) );
	else
		return mu - alpha2 * log( ( 1 - draw ) * ( 1 + alpha1 / alpha2 ) );
}


/*************************************************************
 WARN_DISTR
 *************************************************************/
void lsd::simulation::warn_distr( i_atomT & errCnt, bool & stopErr, const char *d, const char *msg )
{
	if ( ++errCnt < ERR_LIM )	// prevent slow down due to I/O
	{
		plog( "\nWarning: %s in function '%s'", msg, d );
		stopErr = false;
	}
	else
		if ( ! stopErr )
		{
			plog( "\nWarning: too many warnings in function '%s', stop reporting...\n", d );
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
