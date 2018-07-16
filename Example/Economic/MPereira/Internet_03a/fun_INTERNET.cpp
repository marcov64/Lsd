#include "fun_head.h"

/*******************************************************************************

	INTERNET ACCESS SERVICE MARKET
	------------------------------

		Simulation of a provider and consumer market of recurring services.

		Assumed object structure:

							  ROOT
								|
							Internet
							/	|  \
				+-----------+	|	+-----------+
				|				|				|
		NetEqTechnologies	AccProviders	EndUsers
				|				|				|
			Technology		Provider		  User
			<multiple		<multiple		<multiple
			instances>		instances>		instances>
								|
							 Network
							 <multiple
							 instances>

 *******************************************************************************/

#define STRAT_INC	8			// number of incumbent price strategies
#define STRAT_ENTR	8			// number of entrant price strategies
#define RANK_DIM	9			// number of strat. rank dimensions

#define NPROVC	0				// current number of providers index
#define SC		1				// current market share index
#define LKC		2				// current profit margin index
#define SH		3				// historic market share index
#define LKH		4				// historic profit margin index
#define NUSERA	5				// accumulated total # of users index
#define QA		6				// accumulated # of users index
#define LA		7				// accumulated profits index
#define KA		8				// accumulated capital index


// global strategies ranking matrixes 
class rank_strat
{
	public:
	double rsinc[RANK_DIM][STRAT_INC+1],
			  rsentr[RANK_DIM][STRAT_ENTR+1];
};

MODELBEGIN

/******************************************************************************

	SECTOR INITIAL STATE
	--------------------

 ******************************************************************************/

EQUATION("init") 
/* 
Technical initialization function. It is computed only once and then it is 
transformed in a parameter and never computed again. 
Sets the global pointers, creates objects and initializes them.
NEW OBJECTS EQUATIONS NOT INCLUDED HERE ARE NOT CALCULATED IN T=1!!!
*/ 
	rank_strat *r;
	int i,j;

	r=(rank_strat *)malloc(sizeof(rank_strat));	// allocate a new rank matrix object
	for(i=0;i<RANK_DIM;i++)						// clears rank matrix (accumulators)
	{
		for(j=0;j<=STRAT_INC;j++)				// incumbents' strategies
			r->rsinc[i][j]=0;
		for(j=0;j<=STRAT_ENTR;j++)				// entrant' strategies
			r->rsentr[i][j]=0;
	}
	p->hook=reinterpret_cast<object *>(r);		// save pointer to rank matrix object

	v[6]=V("r0");								// base interest rate
	v[8]=V("Pmax");								// maximum end user price
	v[9]=V("Qmin");								// minimum capacity - initial technology
	v[10]=V("Ptechinc");						// incumbent's technology discount
												// Parameters for objects initialization
	v[11]=V("Nprovinit");						// # of initial providers
	v[14]=V("QMavginit");						// initial capacity average
	v[15]=V("QMvarinit");						// initial capacity variance
	v[21]=V("mLmin");							// min rentability per user target
	v[22]=V("mLmax");							// max rentability per user target
	v[25]=V("mMmin");							// min max network quality target
	v[26]=V("mMmax");							// max max network quality target
	v[27]=V("mQmin");							// min capacity response profile
	v[28]=V("mQmax");							// min capacity response profile
	
	v[16]=V("pop0");							// # of initial users
	v[17]=V("Baver");							// initial average budget
	v[18]=V("Bvar");							// initial budget standard deviation
	v[19]=V("b1min");							// min user price sensitivity
	v[20]=V("b1max");							// max user price sensitivity
	v[33]=V("b3min");							// min user market share sensitivity
	v[34]=V("b3max");							// max user market share sensitivity
	v[29]=V("edmin");							// min user quality evaluation error
	v[30]=V("edmax");							// max user quality evaluation error
	v[31]=V("esmin");							// min user loyalty threshold
	v[32]=V("esmax");							// max user loyalty threshold
	v[39]=V("Tavg");							// average contract duration
	v[40]=V("Tvar");							// contract duration std. deviation
	
	v[12]=V("pincr");							// average period for incremental innovation
	v[13]=V("prad");							// average period for radical innovation

	cur5=SEARCH("Technology");
	v[23]=VS(cur5,"Ptech");						// gets initial technology unit price
	v[41]=VS(cur5,"a");							// gets technology productivity
	WRITES(cur5,"tnextincr",poisson(v[12]));	// next incremental innovation time

	cur6=SEARCH("NetEqTechnologies");
	WRITES(cur6,"tnextrad",poisson(v[13]));		// next radical innovation time

	cur=SEARCH("AccProviders");
	for(v[1]=2;v[1]<=v[11];v[1]++)				// starts from provider #2
		ADDOBJS(cur,"Provider");				// assuming #1 already exists

	v[4]=0;										// no installed capacity yet
	v[5]=0;										// no capital employed yet

	CYCLE(cur1,"Provider")						// initializes every provider
	{
		v[1]=V("genprovID");					// new provider ID
		WRITES(cur1,"provID",v[1]);
		WRITES(cur1,"Pprov",v[8]/2);			// initial price = 50% of max price
		WRITES(cur1,"Pd",v[8]/2);				// initial desired price

		v[3]=floor(norm(v[14],v[15]));			// initial network capacity
		if(v[3]<v[9])
			v[3]=v[9];							// subject to minimum capacity

		v[4]+=v[3];								// accumulated initial installed capacity
		WRITES(cur1,"QM",v[3]);					// initial installed capacity
		WRITES(cur1,"QP",v[3]);					// initial planned capacity

		v[5]+=v[3]*v[23]*(1-v[10]);				// accumulated initial capital
		WRITES(cur1,"K",v[3]*v[23]*(1-v[10]));	// initial capital w/ incumbent discount
		WRITES(cur1,"Kdepr",0);					// no initial depreciated capital
		WRITES(cur1,"I",v[3]*v[23]*(1-v[10]));	// initial investment = initial capital
		WRITES(cur1,"D",0);						// no depreciation
		WRITES(cur1,"F",v[3]*v[23]*(1-v[10]));	// initial debt = initial investment
		WRITES(cur1,"L",0);						// no profit
		WRITES(cur1,"R",0);						// no revenue
		WRITES(cur1,"C",0);						// no cost
		WRITES(cur1,"ce",0);					// no unit cost expectation
		WRITES(cur1,"CM",0);					// no maintenance cost
		WRITES(cur1,"AL",0);					// no accumulated profit
		WRITES(cur1,"AF",0);					// no amortization schedule
		WRITES(cur1,"NF",0);					// no finance needs
		WRITES(cur1,"negcashper",0);			// no negative cash period yet
		WRITES(cur1,"zeroshareper",0);			// no irrelevant share period yet
		WRITES(cur1,"r",v[6]);					// start with base interest rate
		WRITES(cur1,"M",v[26]);					// no user so max quality 
		WRITES(cur1,"Q",0);						// no initial user
		WRITES(cur1,"s",0);						// no initial share
		WRITES(cur1,"gs",0);					// no initial share growth
		WRITES(cur1,"group",1);					// is incumbent
		WRITES(cur1,"tentr",0);					// entry in t=0
		WRITES(cur1,"laststratt0",0);			// last strategy pick period
		WRITES(cur1,"strat",rnd_integer(1,STRAT_INC));	// picks an incumbent strategy
		WRITES(cur1,"mL",UNIFORM(v[21],v[22]));	// average user rentability target
		WRITES(cur1,"mM",UNIFORM(v[25],v[26]));	// network quality target
		WRITES(cur1,"mQ",UNIFORM(v[27],v[28]));	// capacity upgrade response profile
		
		cur2=SEARCHS(cur1,"Network");			// all capacity in initial technology
		WRITES(cur2,"QMtech",v[3]);				// init tech capacity installed
		WRITES(cur2,"Ktech",v[3]*v[23]*(1-v[10]));	// and calculates capital
		WRITES(cur2,"t0tech",0);				//	time of adoption
		WRITES(cur2,"atech",v[41]);				//	productivity
		WRITES(cur2,"depr",0);					// not depreciated
		cur2->hook=cur5;						// saves pointer to technology
	}

	WRITE("pop",v[16]);							// initial population
	WRITE("Pavg",v[8]/2);						// initial average price
	WRITE("Mavg",v[4]);							// initial average quality
	WRITES(cur,"QMT",v[4]);						// initial installed capacity
	WRITES(cur,"KT",v[5]);						// initial employed capital

	cur3=SEARCH("EndUsers");
	for(v[2]=2;v[2]<=v[16];v[2]++)				// starts from user #2
		ADDOBJS(cur3,"User");					// assuming #1 already exists

	CYCLE(cur4,"User")
	{
		v[38]=rnd_integer(1,v[11]);				// choose a provider
		WRITES(cur4,"prov",v[38]);
		cur4->hook=SEARCH_CNDS(cur,"provID",v[38]);	// update pointer to provider
		WRITES(cur4,"B",v[8]/2);				// initial budget = initial prices
		WRITES(cur4,"Puser",v[8]/2);

		v[35]=floor(norm(v[39]+0.5,v[40]));		// draws a contract duration (integer)
		if(v[35]<1)
			v[35]=1;
		WRITES(cur4,"T",v[35]);					// average contract duration

		v[36]=UNIFORM(v[19],v[20]);				// tentative b1 draw
		v[37]=UNIFORM(v[33],v[34]);				// tentative b3 draw
		if((v[36]+v[37])>1)						// if draws out of range
		{
			v[36]=v[36]/(v[36]+v[37]);			// normalize values
			v[37]=v[37]/(v[36]+v[37]);			// letting b2=0
		}
		WRITES(cur4,"b1",v[36]);				// user price sensitivity
		WRITES(cur4,"b2",1-v[36]-v[37]);		// user quality sensitivity
		WRITES(cur4,"b3",v[37]);				// user market share sensitivity
		WRITES(cur4,"ed",UNIFORM(v[29],v[30]));	// user quality evaluation error
		WRITES(cur4,"es",UNIFORM(v[31],v[32]));	// user loyalty threshold
	}

	plog("Simulation initialization complete...\n");

	param=1;									// transform "init" in a parameter  
												// so to not compute again
RESULT(1) 


/******************************************************************************

	DEMAND BEHAVIOR
	---------------

 ******************************************************************************/

/********* DEMAND GROWTH ROUTINES *********/

EQUATION("pop")
/*
Population total (users and wanna be users)
*/
	v[1]=V("gusers");							// customer growth rate
	v[2]=V("pop0");								// initial customer population
	v[3]=V("popmax");							// customer stable population
	v[4]=VL("pop",1);							// previous number of users
	v[5]=V("Baver");							// initial average budget
	v[6]=V("Bvar");								// initial budget standard deviation
	v[21]=V("Bmin");							// initial minimum budget
	v[7]=V("b1min");							// min user price sensitivity
	v[8]=V("b1max");							// max user price sensitivity
	v[9]=V("b3min");							// min user market share sensitivity
	v[10]=V("b3max");							// max user market share sensitivity
	v[11]=V("edmin");							// min user quality evaluation error
	v[12]=V("edmax");							// max user quality evaluation error
	v[13]=V("esmin");							// min user loyalty threshold
	v[14]=V("esmax");							// max user loyalty threshold

	cur=SEARCH("EndUsers");						// get users container pointer

	v[15]=ceil(v[3]*v[2]*exp(v[1]*t)/(v[3]+v[2]*(exp(v[1]*t)-1)));
												// logistic curve growth
	v[16]=v[15]-v[4];							// number of users to create

	for(v[17]=1;v[17]<=v[16];v[17]++)			// create user objects
	{
		cur1=ADDOBJS(cur,"User");

		do
			v[18]=norm(v[5],v[6]);				// draw normal budget
		while(v[18]<v[21]);						// search until above minimum

		WRITELS(cur1,"B",v[18],t-1);			// initial budget
		v[19]=UNIFORM(v[7],v[8]);				// tentative b1 draw
		v[20]=UNIFORM(v[9],v[10]);				// tentative b3 draw
		if((v[19]+v[20])>1)						// if draws out of range
		{
			v[19]=v[19]/(v[19]+v[20]);			// normalize values
			v[20]=v[20]/(v[19]+v[20]);			// letting b2=0
		}
		WRITES(cur1,"b1",v[19]);				// user price sensitivity
		WRITES(cur1,"b2",1-v[19]-v[20]);			// user quality sensitivity
		WRITES(cur1,"b3",v[20]);				// user market share sensitivity
		WRITES(cur1,"ed",UNIFORM(v[11],v[12]));	// user quality evaluation error
		WRITES(cur1,"es",UNIFORM(v[13],v[14]));	// user loyalty threshold
		WRITELS(cur1,"prov",0,t-1);				// no initial provider
	}

	STAT("B");									// gets statistics from all population

RESULT(v[0])


EQUATION("B")
/*
Updade user budget for period
*/
	v[1]=VL("B",1);								// get last period budget
	v[2]=V("gB");								// and demand periodic growth

	v[0]=(1+v[2])*v[1];							// new budget
  
RESULT(v[0])


/********* USER SEARCH ROUTINE *********/

EQUATION("prov")
/*
Defines user provider for period
If current contract not expired, keep current provider
*/
	V("pop");									// make sure population is up to date
		
	v[1]=VL("prov",1);							// gets current provider
	v[2]=V("t0");								// last contract start period
	v[3]=V("T");								// current contract duration
	v[4]=V("B");								// current budget
	v[5]=VL("Pavg",1);							// average market price
	v[6]=V("b1");								// low price sensitivity
	v[12]=V("b2");								// quality sensitivity
	v[7]=V("b3");								// market share sensitivity
	v[8]=V("ed");								// quality error standard deviation
	v[9]=V("es");								// utility tolerance to keep provider
	v[10]=V("Tavg");							// average contract duration
	v[11]=V("Tvar");							// contract duration standard deviation

	v[13]=0;									// assumes change is not needed
			
	if(v[1]==0)									// if no provider, try change
	{			
		v[13]=1;								// 0 - no change / 1 - try change
		v[14]=0;								// no quality
		v[15]=0;								// no price
		v[16]=0;								// best utility so far is zero
		v[0]=0;									// no provider ID
		cur=NULL;								// no firm object
	}
	else
		if((t-v[2])>=v[3])						// if contract expired, try change
		{
			v[13]=1;
			v[14]=VLS(p->hook,"M",1);			// gets last quality available
			v[15]=VS(p->hook,"Pprov");			// current price for existing provider
			v[17]=VLS(p->hook,"s",1);			// last share of existing provider
			
			if(v[15]<=v[4])						// check if current provider is affordable
			{
				v[16]=(1+v[9])*pow(v[5]/v[15],v[6])*pow(v[14],v[12])*pow(v[17],v[7]);
												// required mininum new utility to change
				v[0]=v[1];						// start with current provider
				cur=p->hook;
			}
			else								// has to find a cheaper provider
			{
				v[14]=0;						// same as no provider
				v[15]=0;
				v[16]=0;	
				v[0]=0;
				cur=NULL;
			}
		}										// best utility so far is current provider
		else
			v[0]=v[1];							// just keep current provider

	if(v[13]==1)								// if change requested
	{
		CYCLES(p->up->up,cur1,"Provider")		// scan all providers
		{
			v[18]=VS(cur1,"provID");			// gets provider ID
			v[19]=VS(cur1,"Pprov");				// provider current price
			v[20]=VLS(cur1,"M",1);				// last known quality (precise)
			v[21]=VLS(cur1,"s",1);				// last share of provider
			
			if(v[18]!=v[1])						// it is not the current provider
			{
				v[22]=norm(v[20],v[8]*v[20]);	// so introduce error in quality measure
				if(v[22]<=0)					// but do not accept negative quality
					v[22]=0.01;					// assumes a minimal level to allow start
			}
			else
				v[22]=v[20];					// same provider, no error

			if(v[21]==0)						// if provider has no market share yet
				v[21]=0.01;						// assumes 1% to allow startup

			v[23]=pow(v[5]/v[19],v[6])*pow(v[22],v[12])*pow(v[21],v[7]);
												// calculates utility of provider
			if(v[23]>v[16] && v[19]<=v[4])		// if better enough than current and under budget
			{
				v[15]=v[19];					// new price
				v[16]=v[23];					// new best utility
				v[0]=v[18];						// new provider ID
				cur=cur1;						// new provider object
			}
		}

		WRITE("Puser",v[15]);					// updates user info even if same provider
		p->hook=cur;							// because price may have changed
		v[24]=floor(norm(v[10]+0.5,v[11]));		// draws a contract duration (integer)
		if(v[24]<1)
			v[24]=1;
		WRITE("T",v[24]);						// new duration 

		if(cur!=NULL)							// if a provider was found
			WRITE("t0",t);						// set new contract start
		else		
			WRITE("t0",0);						// clear old provider if none found
	}

RESULT(v[0])


/******************************************************************************

	PRODUCTION & MARKETING BEHAVIOR
	-------------------------------

 ******************************************************************************/

/********* MARKET FORECAST & PRODUCTION PLANNING ROUTINE *********/

EQUATION("QP")
/*
Expected provider's capacity required for the next planning period (based on number of expected users)
Allways use 4 periods averages to plan future
To define investment during the current period
*/
	v[1]=V("Q");								// current period user base
	v[2]=VL("Q",4);								// and last 4 periods
	v[3]=V("Nuser");							// current period total market users
	v[4]=VL("Nuser",2);							// and last 4 periods
	v[5]=VL("Nuser",4);	
	v[6]=V("Tplan");							// planning period
	v[7]=V("mQ");								// response profile
	v[8]=V("s");								// provider market share
	v[9]=V("sinc");								// minimum share to consider as incumbent
	
	if(v[8]>v[9])								// if market share is globally relevant
		v[10]=v[8]*(v[3]-v[5])/4;				// global market is the reference
	else										// but for an small player
		v[10]=(v[1]-v[2])/4;					// relevant is its own growth

	if((v[3]-v[4])<=(v[4]-v[5]))				// if growth is deaccelerating
		v[10]=v[10]/2;							// reduce growth estimate by 50%

	v[11]=floor(v[7]*v[10]);			 		// project past growth into future
												// according to providers aggressivity
	if(v[11]<0)									// if even so negative perspective
		v[11]=0;								// expect only holding user base

	v[0]=v[1]+v[11]*v[6];						// planned capacity for Tplan periods
		
RESULT(v[0])


/********* INVESTMENT & TECHNOLOGICAL SEARCH DECISION ROUTINES *********/

EQUATION("I")
/*
Gross investment for period
Creates new network objects for provider as required
*/
	v[1]=VL("QM",1);							// previous period installed capacity
	v[2]=V("QP");								// expected capacity required for period
	v[3]=V("D");								// depreciation for period
	v[4]=V("toptech");							// current network technology
	cur=SEARCH_CNDS(p->up->up,"techID",v[4]);	// get current technology object
	v[5]=VS(cur,"Ptech");						// technology current price
	v[6]=VS(cur,"Qmin");						// minimum capacity step for technology
	v[15]=VS(cur,"a");							// technology productivity
	v[7]=V("tentr");							// period of entry in market
	v[8]=V("Tplan");							// planning period
	v[9]=V("Ptechinc");							// incumbent technology discount
	v[10]=V("group");							// provider social group
	v[11]=V("mM");								// provider quality target

	if(fmod(v[7]-(double)t,v[8])==0)			// if in planning period
		v[12]=ceil(v[11]*v[2]-v[1]+v[3]); 		// calculates ideal net capacity to acquire
	else
		v[12]=0;								// else do nothing
		
	if(v[12]<=0)								// if reduction is required
		v[12]=0;								// no reduction is viable
	else		
		if(v[12]<v[6])							// if net add capacity below minimum capacity
			v[12]=v[6];							// increases net capacity
		
	if(v[10]==1)								// if incumbent
		v[0]=v[12]*v[5]*(1-v[9]);				// calculates discounted investment value
	else		
		v[0]=v[12]*v[5];						// calculates full investment value
		
	if(v[12]>0)									// if more capacity is needed
	{		
		cur1=NULL;								// assumes toptech is not yet installed
		CYCLE(cur2,"Network")					// scans existing network technologies
		{		
			v[13]=VS(cur2,"tech");				// gets ID
			if(v[13]==v[4])		
				cur1=cur2;						// if found current tech
		}

		if(cur1!=NULL)
		{										// technology exists
			v[16]=VS(cur1,"atech");				// current productivity
			v[17]=VS(cur1,"QMtech");			// current installed capacity
			INCRS(cur1,"QMtech",v[12]);			// update technology capacity
			INCRS(cur1,"Ktech",v[0]);			// total capital in tech
			WRITES(cur1,"atech",(v[16]*v[17]+v[15]*v[12])/(v[17]+v[12]));
												// and weighted average productivity
		}
		else
		{
			cur1=ADDOBJ("Network");				// creates a new network technology for provider
			WRITES(cur1,"tech",v[4]);			// tech ID
			WRITES(cur1,"QMtech",v[12]);		// new capacity
			WRITES(cur1,"Ktech",v[0]);			// and capital
			WRITES(cur1,"t0tech",(double)t);	//	time of adoption
			WRITES(cur1,"atech",v[15]);			//	productivity
			WRITES(cur1,"depr",0);				// not deprecated
			cur1->hook=cur;						// saves pointer to technology
		}		

		CYCLE_SAFE(cur3,"Network")				// delete deprecated technologies
		{
			v[14]=VS(cur3,"depr");
			if(v[14]==1)
				DELETE(cur3);
		}
	}

RESULT(v[0])


EQUATION("D")
/*
Depreciation for the period
*/
	v[1]=VL("r",1);								// provider interest rate (last period)
	v[2]=V("toptech");							// current network technology
	cur=SEARCH_CNDS(p->up->up,"techID",v[2]);	// get current technology object
	v[3]=VS(cur,"Ptech");						// technology current price
	v[4]=VS(cur,"cmtech");						// technology current maintenance cost
	v[5]=V("Ptechinc");							// incumbent technology discount
	v[6]=V("group");							// provider social group
	v[7]=VS(p->up->up,"Ttechdepr");				// maximum technology lifespan

	if(v[6]==1)									// if incumbent
	{
		v[3]*=(1-v[5]);							// considers technology discount
		v[4]*=(1-v[5]);
	}

	v[0]=0;										// no depreciation accrued yet

	CYCLE(cur,"Network")						// scans all network technologies
	{
		v[8]=VS(cur,"QMtech");					// technology's installed capacity
		v[9]=VS(cur,"Ktech");					// capital in technology
		v[10]=VS(cur->hook,"cmtech");			// technology maintenance cost
		v[11]=VS(cur,"t0tech");					// network creation time
		v[12]=VS(cur,"tech");					// technology ID

		if(v[6]==1)								// if incumbent
			v[10]*=(1-v[5]);					// considers technology discount

		v[13]=(double)t-v[11];					// calculates network age

		v[15]=0;
		for(v[14]=1;v[14]<=(v[7]-v[13]);v[14]++)// calculates NPV factor for remaining life
			v[15]+=1/pow(1+v[1],v[14]);

		v[16]=v[10]*v[15];						// calculates old technology maintenance NPV
		v[17]=v[4]*v[15];						// calculates new technology maintenance NPV

		if(v[12]<v[2] && (v[13]>=v[7] || v[16]-v[17]>=v[3]))// deprecate if newer technology available
												// if equipment is too old
		{										// or maintenance savings pay replacement cost
			v[0]+=v[8];							// replaces all capacity using this technology
			WRITES(cur,"depr",1);				// mark technology object for deletion
			INCR("Kdepr",v[9]);					// write-off capital
		}
	}

RESULT(v[0])


/********* PRICE DECISION ROUTINES *********/

EQUATION("Pd")
/*
Desired price for provider
*/
	v[1]=V("mL");								// provider's target markup
	v[2]=V("ce");								// expected unit costs
	v[3]=V("ke");								// expected unit capital

	v[0]=v[1]*v[3]+v[2];						// takes minimum profitability
												// or market price
RESULT(v[0])


EQUATION("Pprov")
/*
Effective current period price for provider
Different pricing strategies are applicable, according to provider defined strategy
Price strategy includes adjustments in network quality targets
*/
	v[1]=VL("s",1);								// past share
	v[2]=VL("gs",1);							// past share growth
	v[3]=VL("gs",2);
	v[4]=V("Pd");								// provider's target markup price
	v[5]=VL("Pprov",1);							// last period price
	v[6]=V("strat");							// strategy profile
	v[7]=V("Pstep");							// price change step
	v[8]=VL("cTavg",1);							// market average cost per user
	v[9]=V("Pmax");								// maximum acceptable price
	v[10]=V("gssens");							// market share growth sensitivity 
	v[11]=V("ce");								// expected unit cost
	v[12]=V("tentr");							// entry period
	v[13]=VL("K",1);							// employed capital in t-1
	v[14]=VL("Q",1);							// total users in t-1
	v[15]=V("Tavg");							// average contract time
	v[16]=VL("Pavg",1);							// previous period price average
	v[17]=VL("Mavg",1);							// previous period quality average
	v[18]=VL("r",1);							// previous period interest rate
	v[19]=V("mM");								// provider's quality target
	v[20]=V("mMmin");							// minimum target quality
	v[21]=V("mMmax");							// maximum target quality
	v[22]=VL("Pavginc",1);						// incumbents' price average
	v[23]=VL("Pavgentr",1);						// entrant's price average
	v[24]=VL("Mavginc",1);						// incumbents' quality average
	v[25]=VL("Mavgentr",1);						// entrants' quality average

	if(v[4]==0)									// if no desired price reference
		v[4]=v[16];								// assumes average market price
	if(v[11]==0)								// if no cost reference
		v[11]=v[16];							// assumes average market price
	if(v[22]==0)								// if no incumbent price reference
		v[22]=v[16];							// assumes general market avg. prices
	if(v[23]==0)								// if no entrant price reference
		v[23]=v[16];							// assumes general market avg. prices
	if(v[24]==0)								// if no incumbent quality reference
		v[24]=v[17];							// assumes general market avg. quality
	if(v[25]==0)								// if no entrant quality reference
		v[25]=v[17];							// assumes general market avg. quality

	v[0]=v[5];									// in principle keep price
	v[26]=v[19];								// and quality

	switch((int)v[6])							// calculates price according to strategy
	{
		case 1:									// "smart" strategy #1/11
		case 11:								// adjusts price for growth
			if(v[2]>v[10])						// is share growing?
			{									// then target is desired price
				if(v[5]>((1+v[7])*v[4]))		// if beyond target
					v[0]=(1-v[7])*v[5];			// reduce price
				if(v[5]<((1-v[7])*v[4]))		// if below target
					v[0]=(1+v[7])*v[5];			// increase price
			}
			if(v[2]<(-v[10]))					// is share shrinking?
			{									// then target is cost
				if(v[5]>((1+v[7])*v[11]))		// if beyond target
					v[0]=(1-v[7])*v[5];			// reduce price
				if(v[5]<((1-v[7])*v[11]))		// if below target
					v[0]=(1+v[7])*v[5];			// increase price
			}
			break;

		case 2:									// "alt smart" strategy #2/12
		case 12:								// adjusts price & quality for growth
			if(v[2]>v[10])						// is share growing?
			{									// then target is desired price
				if(v[5]>((1+v[7])*v[4]))		// if beyond target
					v[0]=(1-v[7])*v[5];			// reduce price
				if(v[5]<((1-v[7])*v[4]))		// if below target
					v[0]=(1+v[7])*v[5];			// increase price
				v[26]=(1-v[7])*v[19];			// and reduce quality target
			}
			if(v[2]<(-v[10]))					// is share shrinking?
			{									// then target is cost
				if(v[5]>((1+v[7])*v[11]))		// if beyond target
					v[0]=(1-v[7])*v[5];			// reduce price
				if(v[5]<((1-v[7])*v[11]))		// if below target
					v[0]=(1+v[7])*v[5];			// increase price
				v[26]=(1+v[7])*v[19];			// and increase quality target
			}
			break;

		case 3:									// alternative strategy #3
												// keeps price = average inc. price
			if(v[5]>((1+v[7])*v[22]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[22]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			break;

		case 13:								// alternative strategy #13
												// valid for incumbent & entrant
			if(v[5]>((1+v[7])*v[23]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[23]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			break;

		case 4:									// alternative strategy #4
												// keeps price = average inc. price
			if(v[5]>((1+v[7])*v[22]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[22]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			v[26]=v[24];						// and quality = average inc. quality
			break;

		case 14:								// alternative strategy #14
												// valid for incumbent & entrant
												// keeps price = average entr. price
			if(v[5]>((1+v[7])*v[23]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[23]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			v[26]=v[25];						// and quality = average entr. quality
			break;

		case 5:									// alternative strategy #5/15
		case 15:								// keeps price = average mkt. price
			if(v[5]>((1+v[7])*v[16]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[16]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			break;

		case 6:									// alternative strategy #6/16
		case 16:								// keeps price = average mkt. price
			if(v[5]>((1+v[7])*v[16]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[16]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			v[26]=v[17];						// and quality = average quality
			break;

		case 7:									// alternative strategy #7/17
		case 17:								// keeps price = desired
			if(v[5]>((1+v[7])*v[4]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[4]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			break;

		case 8:									// alternative strategy #8/18
		case 18:								// keeps price = max price
			if(v[5]>((1+v[7])*v[9]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[9]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			v[26]=v[21];						// and quality = max quality
			break;

		case 9:									// alternative strategy #9/19
		case 19:								// keeps price = average mkt. cost
			if(v[5]>((1+v[7])*v[8]))			// if beyond target
				v[0]=(1-v[7])*v[5];				// reduce price
			if(v[5]<((1-v[7])*v[8]))			// if below target
				v[0]=(1+v[7])*v[5];				// increase price
			v[26]=v[20];						// and quality = min quality
			break;

	}

	if(v[0]>v[9])
		v[0]=v[9];								// price cap
	if(v[26]>v[21])								// quality cap
		v[26]=v[21];
	if(v[26]<v[20])								// quality floor
		v[26]=v[20];

	WRITE("mM",v[26]);							// update quality target

RESULT(v[0])


/********* PRODUCTION & MARKETING SUPPORT INFORMATION ROUTINES *********/

EQUATION("Q")
/*
Calculates provider's number of users for the period
*/
	v[0]=0;										// reset found users for provider
	v[1]=V("provID");							// provider ID to search for

	CYCLES(p->up->up,cur,"User")				// scans all users
	{
		v[2]=VS(cur,"prov");					// gets user provider
		
		if(v[2]==v[1])							// counts if same provider
			v[0]++;
	}
	
RESULT(v[0])


EQUATION("Qmavg")
/*
Provider user base moving average (4 period)
*/
	v[1]=VL("Q",1);								// total (4 periods)
	v[2]=VL("Q",2);
	v[3]=VL("Q",3);
	v[4]=VL("Q",4);

	v[0]=(v[1]+v[2]+v[3]+v[4])/4;
	
RESULT(v[0])


EQUATION("QM")
/*
Calculates provider total installed network capacity
*/
	V("I");										// ensure QMtech was updated by investment

	v[0]=SUM("QMtech");							// sum capacity for all network objects

RESULT(v[0])


EQUATION("QMT")
/*
Total installed capacity in internet access segment
*/
	v[0]=SUM("QM");								// sum QM for all provider objects

RESULT(v[0])


EQUATION("M")
/*
Calculates provider quality index in the period
*/
	v[1]=V("Q");								// number of user in the period
	v[2]=V("QM");								// network capacity for the period
	v[3]=V("q");								// quality fixed parameter
	v[4]=V("mMmax"); 							// maximum target quality

	if(v[1]==0)									// if provider has no user
		v[0]=v[4];								// then quality is maximum
	else
		v[0]=pow(v[2]/v[1],v[3]);				// calculates quality

RESULT(v[0])


EQUATION("Mavg")
/*
Market's network quality index weighted average in period
*/
	v[1]=V("Nuser");							// total number of users in period
	v[2]=V("QMT");								// total network capacity in period
	v[3]=V("q");								// quality fixed parameter

	if(v[1]==0)
		v[0]=v[2];								// quality same as network capacity if no user
	else
		v[0]=pow(v[2]/v[1],v[3]);				// calculates quality

RESULT(v[0])


EQUATION("Mavginc")
/*
Incumbents' network quality index weighted average in period
*/
	v[1]=V("q");								// quality fixed parameter

	v[2]=0;										// total users accumulator
	v[3]=0;										// total network capacity
	v[4]=0;										// total incumbents

	CYCLE(cur,"Provider")
	{
		v[5]=VS(cur,"group");					// get provider group
		if(v[5]==1)								// if incumbent
		{
			v[6]=VS(cur,"Q");					// get provider users
			v[7]=VS(cur,"QM");					// get provider price
			v[2]+=v[6];							// update users
			v[3]+=v[7];							// update network capacity
			v[4]++;								// increment providers
		}
	}

	if(v[2]==0)									// no users yet
		if(v[4]==0)								// no providers yet
			v[0]=0;
		else
			v[0]=pow(v[3]/v[4],v[1]);			// non weighted average
	else                                        
		v[0]=pow(v[3]/v[2],v[1]);				// weighted average

RESULT(v[0])


EQUATION("Mavgentr")
/*
Entrants' network quality index weighted average in period
*/
	v[1]=V("q");								// quality fixed parameter

	v[2]=0;										// total users accumulator
	v[3]=0;										// total network capacity
	v[4]=0;										// total entrants

	CYCLE(cur,"Provider")
	{
		v[5]=VS(cur,"group");					// get provider group
		if(v[5]!=1)								// if incumbent
		{
			v[6]=VS(cur,"Q");					// get provider users
			v[7]=VS(cur,"QM");					// get provider price
			v[2]+=v[6];							// update users
			v[3]+=v[7];							// update network capacity
			v[4]++;								// increment providers
		}
	}

	if(v[2]==0)									// no users yet
		if(v[4]==0)								// no providers yet
			v[0]=0;
		else
			v[0]=pow(v[3]/v[4],v[1]);			// non weighted average
	else
		v[0]=pow(v[3]/v[2],v[1]);				// weighted average

RESULT(v[0])


EQUATION("Nmavg")
/*
Industry total users moving average (4 period)
*/
	v[1]=VL("Nuser",1);							// total users (5 periods)
	v[2]=VL("Nuser",2);
	v[3]=VL("Nuser",3);
	v[4]=VL("Nuser",4);
	
	v[0]=(v[1]+v[2]+v[3]+v[4])/4;

RESULT(v[0])


EQUATION("Nuser")
/*
Total users in period
*/
	v[0]=SUM("Q");								// sum Q for all provider objects

RESULT(v[0])


EQUATION("Pavg")
/*
Calculate the average weighted price for period
*/
	v[1]=V("Nuser");							// total users
	v[2]=SUM("Pprov");							// sum of prices
	v[3]=V("Nprov");							// current number of providers
	v[4]=WHTAVE("Pprov","Q");					// price * number of users

	if(v[1]==0)									// no users yet
		v[0]=v[2]/v[3];							// non weighted average
	else	
		v[0]=v[4]/v[1];							// weighted average

RESULT(v[0])


EQUATION("Pavginc")
/*
Calculate the average incumbents' weighted price for period
*/
	v[1]=0;										// total users accumulator
	v[2]=0;										// total price
	v[3]=0;										// total providers
	v[4]=0;										// total price*users

	CYCLE(cur,"Provider")
	{
		v[5]=VS(cur,"group");					// get provider group
		if(v[5]==1)								// if incumbent
		{
			v[6]=VS(cur,"Q");					// get provider users
			v[7]=VS(cur,"Pprov");				// get provider price
			v[1]+=v[6];							// update users
			v[2]+=v[7];							// update prices
			v[4]+=v[7]*v[6];					// update price*users
			v[3]++;								// increment providers
		}
	}

	if(v[1]==0)									// no users yet
		if(v[3]==0)								// no providers yet
			v[0]=0;
		else
			v[0]=v[2]/v[3];						// non weighted average
	else	
		v[0]=v[4]/v[1];							// weighted average

RESULT(v[0])


EQUATION("Pavgentr")
/*
Calculate the average entrants' weighted price for period
*/
	v[1]=0;										// total users accumulator
	v[2]=0;										// total price
	v[3]=0;										// total providers
	v[4]=0;										// total price*users

	CYCLE(cur,"Provider")
	{
		v[5]=VS(cur,"group");					// get provider group
		if(v[5]!=1)								// if incumbent
		{
			v[6]=VS(cur,"Q");					// get provider users
			v[7]=VS(cur,"Pprov");				// get provider price
			v[1]+=v[6];							// update users
			v[2]+=v[7];							// update prices
			v[4]+=v[7]*v[6];					// update price*users
			v[3]++;								// increment providers
		}
	}

	if(v[1]==0)									// no users yet
		if(v[3]==0)								// no providers yet
			v[0]=0;
		else
			v[0]=v[2]/v[3];						// non weighted average
	else	
		v[0]=v[4]/v[1];							// weighted average

RESULT(v[0])


EQUATION("s")
/*
Provider's market share
*/
	v[1]=V("Q");								// # of provider users
	v[2]=V("Nuser");							// total users

	if(v[2]!=0)									// if no user yet, ms=0
		v[0]=v[1]/v[2];
	else
		v[0]=0;

RESULT(v[0])


EQUATION("smavg")
/*
Provider's market share moving average (4 period)
*/
	v[1]=VL("s",1);								// shares (4 periods)
	v[2]=VL("s",2);
	v[3]=VL("s",3);
	v[4]=VL("s",4);

	v[0]=(v[1]+v[2]+v[3]+v[4])/4;
	
RESULT(v[0])


EQUATION("gs")
/*
Provider's market share growth
*/
	v[1]=V("s");								// current share
	v[2]=VL("s",1);								// previous share

	if(v[2]!=0)
		v[0]=(v[1]-v[2])/v[2];
	else
		if(v[1]==0)								// no growth
			v[0]=0;
		else
			v[0]=1;								// conventional infinite growth

RESULT(v[0])


/******************************************************************************

	STRATEGY VALUATION & ADAPTATION
	-------------------------------

 ******************************************************************************/

/********* STRATEGY SELECTION ROUTINE *********/

EQUATION("strat")
/*
Pricing & quality strategy used by provider
(0X: entrant strategies, 1X: incumbent strategies)
*/
	int strat;

	V("topstrat");								// updates rank matrix

	rank_strat *r=reinterpret_cast<rank_strat *>(p->up->up->hook);
												// get pointer to rank matrix object

	v[1]=VL("strat",1);							// running strategy
	v[2]=V("laststratt0");						// last strategy change
	v[3]=V("nstratmin");						// minimum time with strategy 
	v[4]=V("group");							// current social group
	v[5]=V("smavg");							// average market share
	v[6]=V("LKmavg");							// average profit margin
	v[7]=VL("r",1);								// provider cost of oportunity
	v[8]=VL("negcashper",1);					// negative cash periods count
	v[9]=VL("zeroshareper",1);					// irrelevant share periods count
	v[10]=V("mL");								// target margin (RoI)
	v[17]=V("nexit");							// maximum bad periods before exit 

	if(v[4]==0 && v[1]<10)						// if changed group
		v[1]+=10;								// update incumbent to entrant
	if(v[4]==1 && v[1]>=10)
			v[1]-=10;							// or entrant to incumbent

	v[11]=v[8]+v[9];							// "bad results" index

	if(((double)t-v[2])>=v[3] && v[6]<v[10])	// if time to change
	{											// & margin (RoI) below target
		v[0]=v[1];								// starting from current strat.
		v[12]=v[5];								// search for better share
		v[13]=max(v[6],v[7]);					// and better margin (RoI) - min=r

		if(v[4]==1)								// if it is an incumbent
		{
			for(strat=1;strat<=STRAT_INC;strat++)	// look in all strategies
			{
				v[14]=r->rsinc[NPROVC][strat];	// # of providers on strategy
				if(v[14]!=0)					// if any provider
				{
					v[15]=r->rsinc[SC][strat]/v[14];	// strategy avg market share
					v[16]=r->rsinc[LKC][strat];	// strategy margin

					if(strat!=(int)v[1] && v[15]>v[12] && v[16]>v[13])
					{							// if better than previous
	v[0]=(double)strat;							// saves current
	v[12]=v[15];
	v[13]=v[16];
					}
				}
			}
			
			if(v[0]==v[1] && v[11]>(v[17]/2))	// if no better current strategy
			{									// and bad situation, try harder
				if(v[8]>1)						// if is a cash problem
					for(strat=1;strat<=STRAT_INC;strat++)	// look in all strategies
					{
	v[16]=r->rsinc[LKH][strat];					// strategy historic margin
	if(strat!=(int)v[1] && v[16]>v[6]) 	 		// picks best option with better RoI
		v[0]=(double)strat;
					}
				else							// or is a market share problem
					for(strat=1;strat<=STRAT_INC;strat++)	// look in all strategies
					{
	v[15]=r->rsinc[SH][strat];					// strategy historic share
	if(strat!=(int)v[1] && v[15]>0)				// picks best option with share>0
		v[0]=(double)strat;
					}
			}
		}
		else									// it is an entrant
		{
			for(strat=1;strat<=STRAT_ENTR;strat++)	// look in all strategies
			{
				v[14]=r->rsentr[NPROVC][strat];	// # of providers on strategy
				if(v[14]!=0)					// if any provider
				{
					v[15]=r->rsentr[SC][strat]/v[14];	// strategy avg market share
					v[16]=r->rsentr[LKC][strat];	// strategy margin

					if((10+strat)!=(int)v[1] && v[15]>v[12] && v[16]>v[13])
					{							// if better than previous
	v[0]=10+(double)strat;						// saves current (entrant 1X)
	v[12]=v[15];
	v[13]=v[16];
					}
				}
			}

			if(v[0]==v[1] && v[11]>(v[17]/2))	// if no better current strategy
			{									// and bad situation, try harder
				if(v[8]>1)						// if is a cash problem
					for(strat=1;strat<=STRAT_ENTR;strat++)	// look in all strategies
					{
	v[16]=r->rsentr[LKH][strat];				// strategy historic margin
	if(strat!=(int)v[1] && v[16]>v[6])		 	// picks best option with better RoI
		v[0]=10+(double)strat;
					}
				else							// or is a market share problem
					for(strat=1;strat<=STRAT_ENTR;strat++)	// look in all strategies
					{
	v[15]=r->rsentr[SH][strat];					// strategy historic share
	if(strat!=(int)v[1] && v[15]>0)				// picks best option with share>0
		v[0]=10+(double)strat;
					}
			}
		}
	}
	else
		v[0]=v[1];								// no change yet	

	if(v[0]!=v[1])								// if changed strategy
		WRITE("laststratt0",(double)t);			// updates marker

RESULT(v[0])


/********* STRATEGY RANKING ROUTINE *********/

EQUATION("topstrat")
/*
Top strategy for period in number of adopters (providers)
(0-9: incumbent's strategies / 11-19: entrant's strategies)
Updates the rank matrix used for strategy selection
*/
	int strat;

	rank_strat *r=reinterpret_cast<rank_strat *>(p->up->hook);
												// get pointer to rank matrix object

	v[1]=V("Nmavg");							// total average users
	
	if(v[1]==0)									// handles initial conditions
		v[1]=1;

	for(strat=1;strat<=STRAT_INC;strat++)		// updates all incumbent'
	{											// strategies ranks
		v[4]=0;									// provider counter
		v[5]=0;									// user counter
		v[6]=0;									// profits accumulator
		v[7]=0;									// capital accumulator

		CYCLE(cur,"Provider")					// scan all providers
		{
			v[8]=VLS(cur,"strat",1);			// checks strategy used
			if((int)v[8]==strat)				// if current strategy
			{
				v[4]++;							// increment counter
				v[5]+=VS(cur,"Qmavg");			// increment customers
				v[6]+=VS(cur,"Lmavg");			// increment profit
				v[7]+=VS(cur,"Kmavg");			// increment capital
			}
		}

		if(v[7]==0)								// handles initial conditions
			v[7]=1;

		r->rsinc[NPROVC][strat]=v[4];			// current # of providers on strategy
		r->rsinc[SC][strat]=v[5]/v[1];			// current strategy market share
		r->rsinc[LKC][strat]=v[6]/v[7];			// current strategy RoI
		r->rsinc[NUSERA][strat]+=v[1];			// accumulated # of total users
		r->rsinc[QA][strat]+=v[5];				// accumulated # of users on strategy
		r->rsinc[LA][strat]+=v[6];				// accumulated strategy profits
		r->rsinc[KA][strat]+=v[7];				// accumulated strategy capital
		r->rsinc[SH][strat]=r->rsinc[QA][strat]/r->rsinc[NUSERA][strat];
												// historic strategy market share
		r->rsinc[LKH][strat]=r->rsinc[LA][strat]/r->rsinc[KA][strat];
												// historic strategy RoI
	}

	for(strat=1;strat<=STRAT_ENTR;strat++)		// updates all entrants'
	{											// strategies ranks
		v[4]=0;									// provider counter
		v[5]=0;									// user counter
		v[6]=0;									// profits accumulator
		v[7]=0;									// capital accumulator

		CYCLE(cur,"Provider")					// scan all providers
		{
			v[8]=VLS(cur,"strat",1);			// checks strategy used
			if((int)v[8]==strat+10)				// if current strategy
			{
				v[4]++;							// increment counter
				v[5]+=VS(cur,"Qmavg");			// increment customers
				v[6]+=VS(cur,"Lmavg");			// increment profit
				v[7]+=VS(cur,"Kmavg");			// increment capital
			}
		}

		if(v[7]==0)								// handles initial conditions
			v[7]=1;

		r->rsentr[NPROVC][strat]=v[4];			// current # of providers on strategy
		r->rsentr[SC][strat]=v[5]/v[1];			// current strategy market share
		r->rsentr[LKC][strat]=v[6]/v[7];		// current strategy RoI
		r->rsentr[NUSERA][strat]+=v[1];			// accumulated # of total users
		r->rsentr[QA][strat]+=v[5];				// accumulated # of users on strategy
		r->rsentr[LA][strat]+=v[6];				// accumulated strategy profits
		r->rsentr[KA][strat]+=v[7];				// accumulated strategy capital
		r->rsentr[SH][strat]=r->rsentr[QA][strat]/r->rsentr[NUSERA][strat];
												// historic strategy market share
		r->rsentr[LKH][strat]=r->rsentr[LA][strat]/r->rsentr[KA][strat];
												// historic strategy RoI
	}

	v[0]=0;										// top strategy
	v[9]=0;										// top providers

	for(strat=1;strat<=STRAT_INC;strat++)		// check incumbent strategies
		if(r->rsinc[NPROVC][strat]>(int)v[9])	// if better than previous
		{
			v[0]=(double)strat;					// saves new best strategy
			v[9]=(double)r->rsinc[NPROVC][strat];
		}

	for(strat=1;strat<=STRAT_ENTR;strat++)		// check entrant strategies
		if(r->rsentr[NPROVC][strat]>(int)v[9])	// if better than previous
		{
			v[0]=(double)strat+10;				// saves new best strategy
			v[9]=(double)r->rsentr[NPROVC][strat];
		}

RESULT(v[0])


/******************************************************************************

	INSTITUTIONAL FEATURES
	----------------------

 ******************************************************************************/

/********* HIERARCHICAL NETWORK FORMATION *********/

EQUATION("group")
/*
Define if a provider is an entrant or an incumbent
*/
	v[1]=VL("group",1);							// previous status
	v[2]=V("smavg");							// average market share
	v[3]=V("groupoutper");						// periods out of group conditions
	v[4]=V("sinc");								// minimum incumbent share
	v[5]=V("Tinc");								// minimum incumbent market time

	v[0]=v[1];									// assumes no group change
	v[6]=0;										// and fulfilling requirements

//	if(v[1]==1 && v[2]<v[4])					// is an incumbent under min share?
//		if(v[3]>v[5])							// if underperform beyond Tinc
//			v[0]=0;								// becomes an "entrant"
//		else									// if not
//			v[6]=v[3]+1;						// increment underperform timer

	if(v[1]==0 && v[2]>v[4])					// is an entrant over min share?
		if(v[3]>v[5])							// if overperform beyond Tinc
			v[0]=1;								// becomes an incumbent
		else									// if not
			v[6]=v[3]+1;						// increment overperform timer

	WRITE("groupoutper",v[6]);					// updates outperformance timer

RESULT(v[0])


/********* NETWORK INVESTMENT SYNCHRONIZATION *********/

EQUATION("perIinc")
/*
Number of periods from last incumbent major investment cycle
(> 50% renewal of networks in 2 periods)
*/
	v[0]=VL("perIinc",1);						// last value
	v[1]=V("techageinc");						// get average age of
	v[2]=VL("techageinc",2);					// incumbent networks
	v[3]=V("prad");								// average period of radical innov.

	if(v[1]<(0.5*v[2]) && v[0]>v[3])			// if there is a strong drop
												// of relevant size
		v[0]=0;									// restart - new cycle
	else
		v[0]++;									// continues in previous cycle

RESULT(v[0])


/******************************************************************************

	TECHNOLOGY INOVATION & PRODUCTIVITY
	-----------------------------------

 ******************************************************************************/

/********* TECHNOLOGICAL ADVANCE ROUTINES *********/

EQUATION("toptech")
/*
Search for the most productive technology
Creates new technologies periodically (random
*/
	v[1]=VL("toptech",1);						// last period top technology
	cur=SEARCH_CND("techID",v[1]);				// get current technology object
	v[2]=VS(cur,"a");							// current technology productivity
	v[3]=V("prad");								// average period of radical innovation
	v[4]=V("vrad");								// s.d. of radical innovation
	v[5]=V("tnextrad");							// period of next radical innovation
	v[6]=V("Nmavg");							// total average number of users
	v[7]=VS(cur,"Qmin");						// current technology minimum scale
	v[8]=V("cm0");
	v[9]=V("pincr");							// average period of incr. innovation
	v[10]=V("pop");								// current population
	v[11]=V("popmax");							// maximum number of users
	v[12]=V("pop0");							// minimum number of users
	v[13]=V("syncIinc");						// incumbent investment synch toggle
	v[14]=V("perIinc");							// current investment period for incumb.
	
	
	v[0]=v[1];									// supposes no innovation

	if(v[5]==(double)t)							// if it is time to innovate

		if(v[13]==1 && v[14]<v[3])				// if incumbent synch applies
			WRITES(cur->up,"tnextrad",(double)t+v[3]-v[14]);
												// next radical innovation time
												// only after incumbents synch
		else
		{
			v[15]=(1-v[10]/v[11])/(1-v[12]/v[11]);	// innovation damping factor
			v[16]=v[4]*v[15];						// innovation fade out as pop grows
			v[17]=norm((1+v[16])*v[2],v[16]*v[2]);	// normal draw of improvement

			if(v[17]>v[2])							// applies innovation if better
			{
				cur1=ADDOBJS(cur->up,"Technology");	// creates a new technology object
													// define new technologies parameters
				v[0]++;
				WRITES(cur1,"techID",v[0]);			// new ID
				WRITES(cur1,"techt0",(double)t);	// launch: current period
				WRITES(cur1,"techt0incr",(double)t);// last incremental innovation
				WRITES(cur1,"tnextincr",(double)t+poisson(v[9]));// next incr. innovation time
				WRITES(cur1,"Qmin",floor(v[7]*v[17]/v[2]));// minimum capacity block
				WRITELS(cur1,"a",v[17],t-1);		// new productivity
				WRITELS(cur1,"Ptech",1/v[17],t-1);	// new price
				WRITELS(cur1,"cmtech",v[8]*(1/v[17]),t-1);	// new maintenance cost
			}

			WRITES(cur->up,"tnextrad",(double)t+poisson(v[3]));
		}											// next radical innovation time

RESULT(v[0])


EQUATION("a")
/*
Technology productivity
Incremmental innovation happens more likely up to the next radical innovation (in average)
*/
	v[1]=VL("a",1);								// previous productivity
	v[2]=V("pincr");							// average period of incr. innovation
	v[3]=V("vincr");							// s.d. of incr. innovation
	v[4]=V("tnextincr");						// period of next incremental innov.
	v[5]=V("techt0");							// technology launch period
	v[6]=V("prad");								// average period of radical innovation

	v[0]=v[1];									// supposes no innovation

	if(v[4]==(double)t)							// if it is time to innovate
	{
		v[7]=5;									// fixed damping factor
		v[8]=v[7]/v[6];							// damping period 2X rad. innov. 
		v[9]=((double)t-v[5]);					// technology lifetime in avg.

		v[10]=v[3]*(1-1/(1+exp(v[7]-v[8]*v[9])));	// damps variance according to age
													// representing trajectory exhausting
		v[11]=norm(v[1],v[10]*v[1]); 				// normal draw of improvement
		if(v[11]>v[1])								// applies innovation if better
		{
			v[0]=v[11];
			WRITE("techt0incr",(double)t);
		}

		WRITE("tnextincr",(double)t+poisson(v[2]));	// next incremental innovation time
	}

RESULT(v[0])


/********* TECHNOLOGY SUPPORT INFORMATION ROUTINES *********/

EQUATION("Ptech")
/*
Technology price per capacity unit
*/
	v[1]=V("a");								// current productivity

	v[0]=1/v[1];

RESULT(v[0])


EQUATION("cmtech")
/*
Maintenance cost per period and per capacity unit
*/
	v[1]=V("Ptech");							// current price
	v[2]=V("cm0");								// maintenance factor

	v[0]=v[2]*v[1];								// new maintenance cost

RESULT(v[0])


EQUATION("techage")
/*
Weighted average age of network technology(ies) employed by provider (firm level)
*/
	V("I");										// ensure investment is allocated
				
	v[1]=0;										// no time*capital yet
	v[2]=0;										// no capital yet

	CYCLE(cur,"Network")
	{
		v[3]=VS(cur,"t0tech");					// network technology start period
		v[4]=VS(cur,"Ktech");					// network capital employed

		v[1]+=(t-v[3])*v[4];					// accumulates time*capital
		v[2]+=v[4];								// accumulates capital
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// weighted average
	else
		v[0]=0;

RESULT(v[0])


EQUATION("techageinc")
/*
Weighted average age of network technology(ies) employed by incumbents (market level)
*/
	v[1]=0;										// no time*capital yet
	v[2]=0;										// no capital yet

	CYCLE(cur,"Provider")
	{
		v[3]=VLS(cur,"techage",1);				// network technology average age
		v[4]=VLS(cur,"K",1);					// network capital employed
		v[5]=VS(cur,"group");					// provider's group
		
		if(v[5]==1)								// is a incumbent?
		{
			v[1]+=v[3]*v[4];					// accumulates time*capital
			v[2]+=v[4];							// accumulates capital
		}
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// weighted average
	else
		v[0]=0;

RESULT(v[0])


EQUATION("atoptech")
/*
Current productivity of most productive technology
*/
	v[1]=V("toptech");							// get top technology
	cur=SEARCH_CND("techID",v[1]);				// get current technology object
	v[0]=VS(cur,"a");							// best technology productivity

RESULT(v[0])


EQUATION("paavg")
/*
Average provider productivity (weighted)
*/
	v[0]=WHTAVE("atech","QMtech");				// weighted productivity average
	v[1]=V("QM");								// total provider capacity
	v[0]/=v[1];									// normalize productivity

RESULT(v[0])


EQUATION("aavg")
/*
Average industry productivity (weighted)
*/
	v[0]=WHTAVE("paavg","QM");					// weighted productivity average
	v[1]=V("QMT");								// total industry capacity
	v[0]/=v[1];									// normalize productivity

RESULT(v[0])



/******************************************************************************

	MARKET ENTRY & EXIT
	-------------------

 ******************************************************************************/

/********* ENTRY AND EXIT DECISION ROUTINES *********/

EQUATION("entry")
/*
Insert new entrants in market
(WRITELS in t-1 is required to all variables so equations are evaluated in t)
*/
	v[1]=V("r0");								// base interest rate
	v[2]=V("k0");								// average entry size
	v[3]=V("Nmavg");							// moving average total users
	v[4]=V("LTmavg");							// moving average profitability
	v[5]=V("KTmavg");							// moving average capital
	v[6]=VL("Pavg",1);							// average price
	v[7]=V("toptech");							// current network technology
	cur=SEARCH_CNDS(p->up,"techID",v[7]);		// get current technology object
	v[8]=VS(cur,"Ptech");						// technology current price
	v[9]=V("pop");								// potential users population
	v[10]=V("emax");							// maximum number of entrants in period
	v[11]=V("mLmin");							// min rentability per user target
	v[12]=V("mLmax");							// max rentability per user target
	v[13]=V("mMmin");							// min max network quality target
	v[14]=V("mMmax");							// max max network quality target
	v[15]=V("mQmin");							// min capacity response profile
	v[16]=V("mQmax");							// min capacity response profile
	v[25]=V("se");								// minimum free share to entry
	v[26]=VL("entry",1);						// past periods entries
	v[27]=VL("entry",2);
	v[28]=VL("entry",3);
	v[29]=V("Temin");							// minimum time between entries
	v[32]=V("Pstep");							// price change standard step
	v[33]=V("popmax");							// maximum number of users
	v[34]=V("pop0");							// minimum number of users
	v[35]=V("tlastentr");						// last entry time

	if(v[3]==0)									// avoids initial division by zero
		v[3]=1;									// supposes 1 user

	v[36]=v[26]+v[27]+v[28];					// total past 3 periods entries
	v[37]=(1-v[9]/v[33])/(1-v[34]/v[33]);		// market maturity damping factor

	v[0]=0;										// no entry so far
	v[17]=0;									// no entrant capital added yet

	if(((double)t-v[35])>=v[29])				// if it is time to try entry
		if((1-v[3]/v[9])>v[25] && v[36]==0) 	// if there are unserved users
			v[20]=1;							// try to enter anyway in none did
		else									// if not
			v[20]=round(UNIFORM(0,1)*v[37]);	// draws entry decision
	else
		v[20]=0;								// not time to enter

	while(v[20]==1 && v[0]<v[10])				// while entry succeeds (limited to emax)
	{
		v[21]=V("KeT");							// this period current entrant capital
		v[23]=floor((v[21]-v[17])/v[8]);		// capacity added by current entrant
		v[17]=v[21];							// update entrant capital added
		v[18]=v[4]/(v[5]+v[21]);				// current L/K
		v[19]=1-v[3]/v[9]-(v[0]+v[36])*v[2];	// current 1-N/pop (accounting for current 
												// round plus 3 past periods new entrants)
		if(v[18]>v[1] || v[19]>v[25])			// if market profitable or there
		{		   								// are unserved users, enter market
			cur1=ADDOBJ("Provider");			// creates provider object
			v[22]=V("genprovID");				// gets new provider ID
			WRITES(cur1,"provID",v[22]);
			WRITELS(cur1,"Pprov",v[6]*(1-v[32]),t-1);	// initial price below market average
			WRITELS(cur1,"Pd",v[6]*(1-v[32]),t-1);	// initial desired price	
			WRITELS(cur1,"QM",v[23],t-1);		// initial installed capacity
			WRITELS(cur1,"QP",v[23],t-1);		// initial planned capacity
			WRITELS(cur1,"K",v[23]*v[8],t-1);	// initial capital
			WRITELS(cur1,"Kdepr",0,t-1);		// no initial depreciated capital
			WRITELS(cur1,"I",v[23]*v[8],t-1);	// initial investment 
			WRITELS(cur1,"D",0,t-1);			// no depreciation
			WRITELS(cur1,"F",v[23]*v[8],t-1);	// initial debt = initial investment
			WRITELS(cur1,"L",0,t-1);			// no profit
			WRITELS(cur1,"R",0,t-1);			// no revenue
			WRITELS(cur1,"C",0,t-1);			// no cost
			WRITELS(cur1,"ce",0,t-1);			// no unit cost expectation
			WRITELS(cur1,"CM",0,t-1);			// no maintenance cost
			WRITELS(cur1,"AL",0,t-1);			// no accumulated profit
			WRITELS(cur1,"AF",0,t-1);			// no amortization schedule
			WRITELS(cur1,"NF",0,t-1);			// no finance needs
			WRITELS(cur1,"negcashper",0,t-1);	// no negative cash period yet
			WRITELS(cur1,"zeroshareper",0,t-1);	// no irrelevant share period yet
			WRITELS(cur1,"r",v[1],t-1);			// start with base interest rate
			WRITELS(cur1,"M",v[14],t-1);		// no user so max quality
			WRITELS(cur1,"Q",0,t-1);			// no initial user
			WRITELS(cur1,"s",0,t-1);			// no initial share
			WRITELS(cur1,"gs",0,t-1);			// no initial share growth
			WRITELS(cur1,"group",0,t-1);		// is entrant
			WRITES(cur1,"tentr",t);				// time of entry in market
			WRITES(cur1,"laststratt0",(double)t);	// last strategy pick period
			WRITELS(cur1,"strat",rnd_integer(11,10+STRAT_ENTR),t-1);// picks entrant strategy
			WRITES(cur1,"mL",UNIFORM(v[11],v[12]));	// average user rentability target
			WRITES(cur1,"mM",UNIFORM(v[13],v[14]));	// network quality target
			WRITES(cur1,"mQ",UNIFORM(v[15],v[16]));	// capacity upgrade response profile

			v[24]=0;							// counts networks found
			CYCLE_SAFES(cur1,cur2,"Network")	// removes existing networks
			{									// except first one
				v[24]++;						// one more network
				if(v[24]>1)						// keeps only first network object
					DELETE(cur2);		
				else		
					cur3=cur2;					// saves first network object
			}

			WRITES(cur3,"tech",v[7]);			// current technology
			WRITES(cur3,"QMtech",v[23]);		// init tech capacity installed
			WRITES(cur3,"Ktech",v[23]*v[8]);	// and calculates capital
			WRITES(cur3,"t0tech",t-1);			//	time of adoption
			WRITES(cur3,"depr",0);				// not depreciated
			cur3->hook=cur;						// saves pointer to technology
		
			WRITE("tlastentr",(double)t);		// saves time of entry
			v[0]++;								// one more entry
		}
		else
			v[20]=0;
	}

RESULT(v[0])


FUNCTION("KeT")
/*
Total capital committed by entrants in period
Can be updated several times in a single period, if more than a company decides to enter the market.
*/
	v[1]=V("firstKeT");							// get first entrant flag
	v[2]=VL("Nuser",1);							// existing users in t-1
	v[3]=V("k0");								// initial capacity multiplier
	v[4]=V("toptech");							// current network technology
	cur=SEARCH_CNDS(p->up,"techID",v[4]);		// get current technology object
	v[5]=VS(cur,"Ptech");						// technology current price
	v[6]=VS(cur,"Qmin");						// minimum capacity

	if(v[1]==0)									// if first entrant in period
	{
		WRITEL("firstKeT",1,t);					// indicates it
		v[0]=0;									// no entrant committed capital yet
	}
	else
		v[0]=VL("KeT",1);						// capacity committed until now

	v[7]=norm(v[3]*v[2]*v[5],v[3]*v[2]*v[5]);	// total entrant capital
	if(v[7]<(v[6]*v[5]))
		v[7]=v[6]*v[5];							// respecting minimum technology scale

	v[0]+=v[7];

RESULT(v[0])


EQUATION("exit")
/*
Remove failing providers from market
*/
	v[1]=V("nexit");							// max # of bad periods

	v[0]=0;										// nothing done yet
	v[2]=0;										// firm count

	CYCLE(cur,"Provider")						// count providers before exits
		v[2]++;
	
	CYCLE_SAFE(cur,"Provider")					// scans all existing providers
	{
		v[3]=VS(cur,"provID");					// gets ID
		v[4]=VS(cur,"negcashper");				// consecutive negative cash periods
		v[5]=VS(cur,"zeroshareper");			// consecutive irrelevant share per.

		if(v[4]>=v[1] || v[5]>=v[1])			// if max periods reached
		{
			CYCLES(p->up,cur1,"User")			// scans all users
			{
				v[7]=VS(cur1,"prov");			// gets user provider
		
				if(v[7]==v[3])					// check if current provider's user
				{
					WRITELS(cur1,"prov",0,t-1);	// finishes contract
					WRITES(cur1,"Puser",0);		// so user can search for
					WRITES(cur1,"t0",0);
					cur1->hook=NULL;			// a new provider in t+1
				}
			}
			
			if(v[0]==(v[2]-1))					// checks if all but last firm
			{									// were already deleted
				quit=1;							// if so, simply finish simulation
				plog("\nSimulation early finish: no more firms.\n");
			}
			else
			{
				DELETE(cur);					// delete provider object
				v[0]++;							// one provider less
			}
		}
	}

RESULT(v[0])


/********* ENTRY & EXIT INFORMATION ROUTINES *********/

EQUATION("firstKeT")
/*
Indicates that no first entrant commited capital in period (=0). 
Overwritten to 1 after the first firm commits capital.
*/

RESULT(0)


FUNCTION("genprovID")
/*
Generates a unique provider ID each call
*/

RESULT(CURRENT+1)


EQUATION("Nprov")
/*
Total providers in the beginning of the period (before entry)
*/
	STAT("provID");								// get statistics of providers
												// number in v[0]
RESULT(v[0])


EQUATION("negcashper")
/*
Number of consecutive negative cash periods
*/
	v[1]=VL("negcashper",1);					// Previous # of periods
	v[2]=V("L");								// current period profits
	v[3]=V("AL");								// accumulated profits
		
	v[4]=v[2]+v[3];								// available cash

	if(v[4]>=0)									// if cash positive
		v[0]=0;									// done
	else
		if(v[1]==0)								// if not insolvent until now
			v[0]=1;								// first negative cash period
		else
			v[0]=v[1]+1;						// another negative period

RESULT(v[0])


EQUATION("zeroshareper")
/*
Number of consecutive irrelevant market share periods
*/
	v[1]=VL("zeroshareper",1);					// Previous # of periods
	v[2]=V("s");								// current market share
	v[3]=V("smin");								// minimum share to stay
	
	if(v[2]>v[3])								// if more than minimum share
		v[0]=0;									// keep provider
	else
		if(v[1]==0)								// if not irrelevant until now
			v[0]=1;								// first irrelevant period
		else
			v[0]=v[1]+1;						// another irrelevant period
	
RESULT(v[0])


/******************************************************************************

	FINANCIAL BEHAVIOR
	------------------

 ******************************************************************************/

/********* FINANCE DECISION ROUTINES *********/

EQUATION("NF")
/*
Finance needs in period
*/
	v[1]=V("I");								// investment demand
	v[2]=VL("AL",1);							// accumulated profits till beginning last period
	v[3]=VL("L",1);								// last period profits

	if(v[1]>(v[2]+v[3]))						// if investment needs bigger than own funds
		v[0]=v[1]-(v[2]+v[3]);					// finance difference
	else
		v[0]=0;

RESULT(v[0])


EQUATION("AF")
/*
Debt amortization schedule for the period
Less debt is preferred to accumulate profits
*/
	v[1]=V("I");								// investment demand
	v[2]=VL("AL",1);							// accumulated profits till beginning last period
	v[3]=VL("L",1);								// last period profits
	v[4]=VL("F",1);								// last period debt stock

	if(v[1]>(v[2]+v[3]))						// if investment needs bigger than own funds
		v[0]=0;									// no amortization is possible
	else
		if((v[2]+v[3]-v[1])>v[4])				// if all debt can be paid
			v[0]=v[4];							// pay it
		else
			v[0]=v[2]+v[3]-v[1];				// pay what is possible

RESULT(v[0])


/********* COST EXPECTATION & INTEREST DECISION ROUTINES *********/

EQUATION("ce")
/*
Expected unit costs for the period
*/
	v[1]=VL("C",1);								// previous period total cost
	v[2]=VL("Q",1);								// previous period total users

	if(v[1]!=0 && v[2]!=0)
		v[0]=v[1]/v[2];
	else
		v[0]=0;									// indicates no unit cost expectation

RESULT(v[0])


EQUATION("ke")
/*
Expected unit kapital requirements for the period
*/
	v[1]=VL("K",1);								// previous period total capital in use
	v[2]=VL("Q",1);								// previous period total users

	if(v[1]!=0 && v[2]!=0)
		v[0]=v[1]/v[2];
	else
		v[0]=0;									// indicates no capital cost expectation

RESULT(v[0])


EQUATION("r")
/*
Interest rate applicable to provider
*/
	v[1]=V("r0");								// base interest rate
	v[2]=V("rlev");								// leverage sensitivity
	v[3]=V("rsize");							// firm size sensitivity
	v[4]=VL("F",1);								// last period debt stock
	v[5]=VL("K",1);								// last period capital stock
	v[6]=VL("L",1);								// last period profit
	v[7]=V("rinc");								// incumbent interest reduction
	v[8]=V("group");							// provider social group

	if(v[6]<1)									// prevents invalid argument to log
		v[6]=1;		
		
	if(v[5]==0)									// new firm (?)
		v[0]=v[1];								// conventional rate
	else
		v[0]=v[1]*(1+v[2]*v[4]/v[5])*(1-v[3]*log(v[6]));

	if(v[8]==1)									// if incumbent
		v[0]*=v[7];								// considers interest reduction

	v[0]=min(v[0],1);							// cap interest to 100%

RESULT(v[0])


/********* FINANCIAL SUPPORT INFORMATION ROUTINES *********/

EQUATION("F")
/*
Provider debt stock
*/
	v[1]=VL("F",1);								// debt stock at end of previous period
	v[2]=V("NF");								// new finance needs in period
	v[3]=V("AF");								// past period amortization schedule

	v[0]=v[1]+v[2]-v[3];						// new debt stock

RESULT(v[0])


EQUATION("AL")
/*
Accumulated profits from previous periods (excluding current)
*/
	v[1]=VL("AL",1);							// previous period acctd profits
	v[2]=VL("L",1);								// last period profits
	v[3]=V("NF");								// net new finance needs
	v[4]=V("I");								// investment schedule
	v[5]=V("AF");								// debt amortization schedule

	v[0]=v[1]+v[2]+v[3]-v[4]-v[5];

RESULT(v[0])


EQUATION("K")
/*
Capital in use (operating capacity only)
*/
	V("I");										// ensure period investment done

	v[0]=SUM("Ktech");							// adds up capital in all technologies

RESULT(v[0])


EQUATION("Kmavg")
/*
Provider employed capital moving average (4 period)
*/
	v[1]=VL("K",1);								// total (4 periods)
	v[2]=VL("K",2);
	v[3]=VL("K",3);
	v[4]=VL("K",4);

	v[0]=(v[1]+v[2]+v[3]+v[4])/4;
	
RESULT(v[0])


EQUATION("KT")
/*
Total capital employed in internet access segment
*/
	v[0]=SUM("K");								// sum capital of all providers

RESULT(v[0])


EQUATION("KTmavg")
/*
Industry employed capital moving average (4 period)
*/
	v[1]=VL("KT",1);							// total (4 periods)
	v[2]=VL("KT",2);
	v[3]=VL("KT",3);
	v[4]=VL("KT",4);

	v[0]=(v[1]+v[2]+v[3]+v[4])/4;
	
RESULT(v[0])


EQUATION("R")
/*
Provider's revenue in period
*/
	v[0]=0;
	v[1]=V("provID");							// provider's own ID

	CYCLES(p->up->up,cur,"User")				// scans its users
	{
		v[2]=VS(cur,"prov");					// check if it is a customer
		if(v[2]==v[1])
		{
			v[3]=VS(cur,"Puser");				// user valid price
			v[0]+=v[3];
		}
	}

RESULT(v[0])


EQUATION("C")
/*
Provider total cost in period (includig scale gains, if applicable)
*/
	v[1]=V("CM");								// maintenance costs
	v[2]=V("cf");								// fixed cost ratio (per user)
	v[3]=V("Q");								// current number of users
	v[4]=V("cs");								// scale factor
	v[5]=V("r");								// providers current interest rate
	v[6]=VL("F",1);								// debt stock at end of previous period

	v[0]=pow(v[1]+v[2]*v[3],v[4])+v[5]*v[6];	// total running costs

RESULT(v[0])


EQUATION("cTavg")
/*
Market average cost per user in period
*/
	v[1]=V("Nuser");							// total users in market
	v[2]=SUM("C");								// total costs in all providers

	if(v[1]==0)									// if no user yet
		v[0]=0;
	else
		v[0]=v[2]/v[1];							// caclulates user cost

RESULT(v[0])


EQUATION("CM")
/*
Maintenance costs (no scale considered)
*/
	v[1]=V("Ptechinc");							// incumbent technology discount
	v[2]=V("group");							// provider social group

	v[0]=0;
	CYCLE(cur,"Network")						// calculates maintenance costs
	{											// for all operating technologies
		v[3]=VS(cur,"QMtech");
		v[4]=VS(cur->hook,"cmtech");

		if(v[2]==1)								// if incumbent
			v[4]*=(1-v[1]);						// considers technology discount

		v[0]+=v[3]*v[4];
	}

RESULT(v[0])


EQUATION("L")
/*
Provider profit in period
*/
	v[1]=V("R");								// Revenue in period
	v[2]=V("C");								// Running costs
	
	v[0]=v[1]-v[2];								// Profit in period

RESULT(v[0])


EQUATION("Lmavg")
/*
Provider profits moving average (4 period)
*/
	v[1]=VL("L",1);								// total profits (4 periods)
	v[2]=VL("L",2);
	v[3]=VL("L",3);
	v[4]=VL("L",4);
							
	v[0]=(v[1]+v[2]+v[3]+v[4])/4;
	
RESULT(v[0])


EQUATION("LT")
/*
Total profits in internet access segment
*/
	v[0]=SUM("L");								// sum profit of all providers

RESULT(v[0])


EQUATION("LTmavg")
/*
Industry profits moving average (4 period)
*/
	v[1]=VL("LT",1);							// total profits (4 periods)
	v[2]=VL("LT",2);
	v[3]=VL("LT",3);
	v[4]=VL("LT",4);
							
	v[0]=(v[1]+v[2]+v[3]+v[4])/4;
	
RESULT(v[0])


EQUATION("LK")
/*
Provider return on investment (RoI)
*/
	v[1]=V("L");								// profitability
	v[2]=V("K");								// capital

	if(v[2]==0)
		v[0]=0;
	else
		v[0]=v[1]/v[2];

RESULT(v[0])


EQUATION("LKmavg")
/*
Provider average return on investment (RoI)
*/
	v[1]=V("Lmavg");							// moving average profitability
	v[2]=V("Kmavg");							// moving average capital

	if(v[2]==0)
		v[0]=0;
	else
		v[0]=v[1]/v[2];

RESULT(v[0])


EQUATION("age")
/*
Provider age in market (periods)
*/
	v[1]=V("tentr");							// period of entry

	v[0]=(double)t-v[1];						// age
	
RESULT(v[0])


/******************************************************************************

	REPORTING & ANALYSIS VARIABLES
	------------------------------

 ******************************************************************************/

/********* DEMAND STUFF *********/

EQUATION("G")
/*
Current demand in period
*/
	V("Nuser");									// make sure all users search provider

	v[0]=SUM("Puser");							// adds all users with contracted access
 
RESULT(v[0])


EQUATION("GM")
/*
Maximum demand for period
*/
	V("pop");									// make sure population is up to date

	v[0]=SUM("B");								// adds all available user budgets

RESULT(v[0])


EQUATION("popcov")
/*
% of potential user population covered by access service
*/
	v[1]=V("Nuser");							// total users in period
	v[2]=V("pop");								// total potential user population

	if(v[2]==0)
		v[0]=0;
	else
		v[0]=v[1]/v[2];

RESULT(v[0])



/********* MARKETING & PRODUCTION STUFF *********/

EQUATION("HHIs")
/*
Herfindahl-Hirschman index of the access provider market (market-share)
*/
	v[0]=0;

	CYCLE(cur,"Provider")						// scans all providers
	{
		v[1]=VS(cur,"s");						// get market share
		v[0]+=pow(v[1],2);						// squares sum
	}

RESULT(v[0])


EQUATION("sTinc")
/*
Market share of incumbents
*/
	v[1]=0;										// no user yet
	v[2]=V("Nuser");							// total users

	CYCLE(cur,"Provider")
	{
		v[3]=VS(cur,"Q");						// provider's customers
		v[4]=VS(cur,"group");					// provider's group
		
		if(v[4]==1)								// is an incumbent?
			v[1]+=v[3];							// accumulates users
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// share
	else
		v[0]=0;

RESULT(v[0])


EQUATION("sTentr")
/*
Market share of entrants
*/
	v[1]=0;										// no user yet
	v[2]=V("Nuser");							// total users

	CYCLE(cur,"Provider")
	{
		v[3]=VS(cur,"Q");						// provider's customers
		v[4]=VS(cur,"group");					// provider's group
		
		if(v[4]==0)								// is an entrant
			v[1]+=v[3];							// accumulates users
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// share
	else
		v[0]=0;

RESULT(v[0])


EQUATION("VMRM")
/*
Variance-to-mean ratio of network quality (unweighted)
*/
	STAT("M");

	v[0]=v[2]/v[1];

RESULT(v[0])


EQUATION("VMRP")
/*
Variance-to-mean ratio of available prices (unweighted)
*/
	STAT("Pprov");

	v[0]=v[2]/v[1];

RESULT(v[0])


/********* INSTITUTIONAL STUFF *********/

EQUATION("ageavg")
/*
Average age of providers in market (periods)
*/
	v[1]=V("Nprov");							// number of providers
	v[2]=SUM("age");							// sums all providers' ages

	v[0]=v[2]/v[1];								// average age
	
RESULT(v[0])


EQUATION("ageavgentr")
/*
Average age of entrant providers in market (periods)
*/
	v[1]=0;										// no provider yet
	v[2]=0;										// no accumulated age yet

	CYCLE(cur,"Provider")
	{
		v[3]=VS(cur,"age");						// provider's customers
		v[4]=VS(cur,"group");					// provider's group
		
		if(v[4]==0)								// is an entrant?
		{
			v[1]++;								// one more entrant
			v[2]+=v[3];							// accumulates age
		}
	}

	if(v[1]==0)
		v[0]=0;									// no entrant in market
	else
		v[0]=v[2]/v[1];							// average age
	
RESULT(v[0])


EQUATION("ageavginc")
/*
Average age of incumbent providers in market (periods)
*/
	v[1]=0;										// no provider yet
	v[2]=0;										// no accumulated age yet

	CYCLE(cur,"Provider")
	{
		v[3]=VS(cur,"age");						// provider's customers
		v[4]=VS(cur,"group");					// provider's group
		
		if(v[4]==1)								// is an incumbent?
		{
			v[1]++;								// one more entrant
			v[2]+=v[3];							// accumulates age
		}
	}

	if(v[1]==0)
		v[0]=0;									// no incumbent in market
	else
		v[0]=v[2]/v[1];							// average age
	
RESULT(v[0])


EQUATION("Ninc")
/*
Number of incumbent providers
*/
	v[0]=0;										// no incumbent yet

	CYCLE(cur,"Provider")						// scan all providers
	{                                       
		v[1]=VS(cur,"group");					// checks status
		if(v[1]==1)								// if incumbent
			v[0]++;								// increment counter
	}

RESULT(v[0])


/********* TECHNOLOGY STUFF *********/

EQUATION("techageentr")
/*
Weighted average age of network technology(ies) employed by entrants (market level)
*/
	v[1]=0;										// no time*capital yet
	v[2]=0;										// no capital yet

	CYCLE(cur,"Provider")
	{
		v[3]=VLS(cur,"techage",1);				// network technology average age
		v[4]=VLS(cur,"K",1);					// network capital employed
		v[5]=VS(cur,"group");					// provider's group
		
		if(v[5]==0)								// is an entrant
		{
			v[1]+=v[3]*v[4];					// accumulates time*capital
			v[2]+=v[4];							// accumulates capital
		}
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// weighted average
	else
		v[0]=0;

RESULT(v[0])


/********* FINANCIAL STUFF *********/

EQUATION("ALAK")
/*
Provider total return (accumulated profits) on total investment (incl. deprec.)
*/
	v[1]=V("L");								// profitability
	v[2]=V("AL");								// accumulated profits up to t-1
	v[3]=V("F");								// accumulated debt up to t-1
	v[4]=V("K");								// capital in use
	v[5]=V("Kdepr");							// capital already deprecated

	if((v[4]+v[5])==0)
		v[0]=0;
	else
		v[0]=(v[1]+v[2]-v[3])/(v[4]+v[5]);

RESULT(v[0])


EQUATION("ALAKT")
/*
Industry total return (accumulated profits) on total investment (incl. deprec.)
*/
	v[1]=SUM("L");								// all providers' profitability
	v[2]=SUM("AL");								// accumulated profits up to t-1
	v[3]=SUM("F");								// accumulated debt up to t-1
	v[4]=SUM("K");								// capital in use
	v[5]=SUM("Kdepr");							// capital already deprecated

	if((v[4]+v[5])==0)
		v[0]=0;
	else
		v[0]=(v[1]+v[2]-v[3])/(v[4]+v[5]);

RESULT(v[0])


EQUATION("ALAKTinc")
/*
Incumbents' total return (accumulated profits) on total investment (incl. deprec.)
*/
	v[1]=0;										// no profits yet
	v[2]=0;										// no accumulated profits
	v[3]=0;										// no accumulated debt
	v[4]=0;										// no capital in use
	v[5]=0;										// nocapital already deprecated

	CYCLE(cur,"Provider")
	{
		v[6]=V("L");							// profitability
		v[7]=V("AL");							// accumulated profits up to t-1
		v[8]=V("F");							// accumulated debt up to t-1
		v[9]=V("K");							// capital in use
		v[10]=V("Kdepr");						// capital already deprecated
		v[11]=VS(cur,"group");					// provider's group
		
		if(v[11]==1)							// is an incumbent
		{	
			v[1]+=v[6];							// accumulates profits
			v[2]+=v[7];							// accumulates capital
			v[3]+=v[8];							// accumulates capital
			v[4]+=v[9];							// accumulates capital
			v[5]+=v[10];						// accumulates capital
		}
	}

	if((v[4]+v[5])==0)
		v[0]=0;
	else
		v[0]=(v[1]+v[2]-v[3])/(v[4]+v[5]);

RESULT(v[0])


EQUATION("ALAKTentr")
/*
Entrant's total return (accumulated profits) on total investment (incl. deprec.)
*/
	v[1]=0;										// no profits yet
	v[2]=0;										// no accumulated profits
	v[3]=0;										// no accumulated debt
	v[4]=0;										// no capital in use
	v[5]=0;										// nocapital already deprecated

	CYCLE(cur,"Provider")
	{
		v[6]=V("L");							// profitability
		v[7]=V("AL");							// accumulated profits up to t-1
		v[8]=V("F");							// accumulated debt up to t-1
		v[9]=V("K");							// capital in use
		v[10]=V("Kdepr");						// capital already deprecated
		v[11]=VS(cur,"group");					// provider's group
		
		if(v[11]==0)							// is an entrant
		{	
			v[1]+=v[6];							// accumulates profits
			v[2]+=v[7];							// accumulates capital
			v[3]+=v[8];							// accumulates capital
			v[4]+=v[9];							// accumulates capital
			v[5]+=v[10];						// accumulates capital
		}
	}

	if((v[4]+v[5])==0)
		v[0]=0;
	else
		v[0]=(v[1]+v[2]-v[3])/(v[4]+v[5]);

RESULT(v[0])


EQUATION("HHIK")
/*
Herfindahl-Hirschman index of capital in use by providers
*/
	v[1]=V("KT");								// industry total capital

	v[0]=0;

	if(v[1]!=0)									// if there is capital in industry
		CYCLE(cur,"Provider")					// scans all providers
		{
			v[2]=VS(cur,"K");					// get market share
			v[0]+=pow(v[2]/v[1],2);				// squares sum
		}

RESULT(v[0])


EQUATION("LKTavg")
/*
Industry weighted average return on investment (RoI)
*/
	v[1]=V("LT");								// average profitability
	v[2]=V("KT");								// average capital

	if(v[2]==0)
		v[0]=0;
	else
		v[0]=v[1]/v[2];

RESULT(v[0])


EQUATION("LKTavginc")
/*
Weighted average return on investment (RoI) of incumbents
*/
	v[1]=0;										// no profit yet
	v[2]=0;										// no capital yet

	CYCLE(cur,"Provider")
	{
		v[3]=VS(cur,"L");						// profits
		v[4]=VS(cur,"K");						// network capital employed
		v[5]=VS(cur,"group");					// provider's group
		
		if(v[5]==1)								// is a incumbent
		{
			v[1]+=v[3];							// accumulates profits
			v[2]+=v[4];							// accumulates capital
		}
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// weighted average
	else
		v[0]=0;

RESULT(v[0])


EQUATION("LKTavgentr")
/*
Weighted average return on investment (RoI) of entrants
*/
	v[1]=0;										// no profit yet
	v[2]=0;										// no capital yet

	CYCLE(cur,"Provider")
	{
		v[3]=VS(cur,"L");						// profits
		v[4]=VS(cur,"K");						// network capital employed
		v[5]=VS(cur,"group");					// provider's group
		
		if(v[5]==0)								// is an entrant
		{			
			v[1]+=v[3];							// accumulates profits
			v[2]+=v[4];							// accumulates capital
		}
	}

	if(v[2]!=0)
		v[0]=v[1]/v[2];							// weighted average
	else
		v[0]=0;

RESULT(v[0])


/********* STRATEGY STUFF *********/

EQUATION("printRankMatrix")
/*
Print Rank Matrix in log window if tdebug is different from 0
*/
	rank_strat *r=reinterpret_cast<rank_strat *>(p->up->hook);
												// get pointer to rank matrix object

	v[2]=V("tdebug");							// debug period toggle

	if(v[2]!=0 && fmod(t,v[2])==0)				// rank matrix printout
	{											// debug only
		char texto[1024];
		sprintf(texto,"\nStrategies in t=%d\n",t);
		plog(texto);
		sprintf(texto,"Strat(inc):\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
		plog(texto);
		sprintf(texto,"Nprov:\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\n",
			r->rsinc[NPROVC][1],r->rsinc[NPROVC][2],r->rsinc[NPROVC][3],r->rsinc[NPROVC][4],
			r->rsinc[NPROVC][5],	r->rsinc[NPROVC][6],r->rsinc[NPROVC][7],r->rsinc[NPROVC][8],
			r->rsinc[NPROVC][9]);
		plog(texto);
		sprintf(texto,"ms(c):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsinc[SC][1],r->rsinc[SC][2],r->rsinc[SC][3],r->rsinc[SC][4],r->rsinc[SC][5],
			r->rsinc[SC][6],r->rsinc[SC][7],r->rsinc[SC][8],r->rsinc[SC][9]);
		plog(texto);
		sprintf(texto,"RoI(c):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsinc[LKC][1],r->rsinc[LKC][2],r->rsinc[LKC][3],r->rsinc[LKC][4],r->rsinc[LKC][5],
			r->rsinc[LKC][6],r->rsinc[LKC][7],r->rsinc[LKC][8],r->rsinc[LKC][9]);
		plog(texto);
		sprintf(texto,"ms(h):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsinc[SH][1],r->rsinc[SH][2],r->rsinc[SH][3],r->rsinc[SH][4],r->rsinc[SH][5],
			r->rsinc[SH][6],r->rsinc[SH][7],r->rsinc[SH][8],r->rsinc[SH][9]);
		plog(texto);
		sprintf(texto,"RoI(h):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsinc[LKH][1],r->rsinc[LKH][2],r->rsinc[LKH][3],r->rsinc[LKH][4],r->rsinc[LKH][5],
			r->rsinc[LKH][6],r->rsinc[LKH][7],r->rsinc[LKH][8],r->rsinc[LKH][9]);
		plog(texto);
		sprintf(texto,"Strat(entr):\t11\t12\t13\t14\t15\t16\t17\t18\t19\n");
		plog(texto);
		sprintf(texto,"Nprov:\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\t%1.0f\n",
			r->rsentr[NPROVC][1],r->rsentr[NPROVC][2],r->rsentr[NPROVC][3],r->rsentr[NPROVC][4],
			r->rsentr[NPROVC][5],r->rsentr[NPROVC][6],r->rsentr[NPROVC][7],r->rsentr[NPROVC][8],
			r->rsentr[NPROVC][9]);
		plog(texto);
		sprintf(texto,"ms(c):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsentr[SC][1],r->rsentr[SC][2],r->rsentr[SC][3],r->rsentr[SC][4],r->rsentr[SC][5],
			r->rsentr[SC][6],r->rsentr[SC][7],r->rsentr[SC][8],r->rsentr[SC][9]);
		plog(texto);
		sprintf(texto,"RoI(c):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsentr[LKC][1],r->rsentr[LKC][2],r->rsentr[LKC][3],r->rsentr[LKC][4],r->rsentr[LKC][5],
			r->rsentr[LKC][6],r->rsentr[LKC][7],r->rsentr[LKC][8],r->rsentr[LKC][9]);
		plog(texto);
		sprintf(texto,"ms(h):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsentr[SH][1],r->rsentr[SH][2],r->rsentr[SH][3],r->rsentr[SH][4],r->rsentr[SH][5],
			r->rsentr[SH][6],r->rsentr[SH][7],r->rsentr[SH][8],r->rsentr[SH][9]);
		plog(texto);
		sprintf(texto,"RoI(h):\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\t%2.2f\n",
			r->rsentr[LKH][1],r->rsentr[LKH][2],r->rsentr[LKH][3],r->rsentr[LKH][4],r->rsentr[LKH][5],
			r->rsentr[LKH][6],r->rsentr[LKH][7],r->rsentr[LKH][8],r->rsentr[LKH][9]);
		plog(texto);

		v[0]=1;									// printout in this period
	}
	else
		v[0]=0;

RESULT(v[0])


EQUATION("strat1")
/*
Number of players using price strategy #1
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==1)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat2")
/*
Number of players using price strategy #2
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==2)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat3")
/*
Number of players using price strategy #3
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==3)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat4")
/*
Number of players using price strategy #4
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==4)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat5")
/*
Number of players using price strategy #5
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==5)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat6")
/*
Number of players using price strategy #6
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==6)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat7")
/*
Number of players using price strategy #7
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==7)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat8")
/*
Number of players using price strategy #8
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==8)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat9")
/*
Number of players using price strategy #9
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==9)								// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat11")
/*
Number of players using price strategy #11
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==11)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat12")
/*
Number of players using price strategy #12
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==12)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat13")
/*
Number of players using price strategy #13
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==13)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat14")
/*
Number of players using price strategy #14
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==14)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat15")
/*
Number of players using price strategy #15
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==15)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat16")
/*
Number of players using price strategy #16
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==16)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat17")
/*
Number of players using price strategy #17
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==17)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat18")
/*
Number of players using price strategy #18
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==18)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


EQUATION("strat19")
/*
Number of players using price strategy #19
*/
	v[0]=0;										// no strategy user yet

	CYCLE(cur,"Provider")						// scan all providers
	{
		v[1]=VS(cur,"strat");					// checks strategy used
		if(v[1]==19)							// if right strategy
			v[0]++;								// increment counter
	}

RESULT(v[0])


MODELEND


void close_sim(void)
{
}


