
import java.io.*;
import java.util.Date;


class Firm {
	int id;
	int birth; // birth period
	int generation; // generation=1 for first group of firms else 2, and 3 for daughters
	int whenEntered;
	boolean alive;
	boolean entered;
	boolean diversified;
	boolean adopted;
	boolean daughter;
	boolean mother;
	double initBud;
	double debt; // debt is equal to initbud
	double bud;
	double RDExpenditure;
	double mup;
	double u; // utility of the product. Updated in market procedures
	double profit;
	double utility; // product utility of served user class
	double profitRate; // see bookeeping for definition
	double oldProfitRate;
	double slopeOfProfitRate;
	int numberOfNewSubMarkets;
	double share;
	double experience;
	double averageExperience;
	double ebw; // endogenous band wagon effect (that is cumulated expenditures in marketing)
	Technology tec = new Technology();
	Mix mix = new Mix(); // engineer mix
	ResTeam res = new ResTeam(); // engineers for cheapness and performance
	Product x = new Product();	// x is design
	UserClass servedUserClass = new UserClass(); // servedUserClass is the user class served by the firm
	Standard stan = new Standard(); // standard
	int standard;
	int servedMkt; // 1 if big firms' market 2 if individuals' market
	int numberOfServedSubMarkets;
	int numberOfBreakdowns;
	int numberOfBLReturns;
	double averageAgeOfSubMarkets;

	Firm(int n, boolean ALIVE) {
		id = n;
		alive = ALIVE;
	}

 	Firm(int a, Mix b) {
		id = a;
		mix = b;
	}

	// constructor used by ?
	Firm(int ID,int BIRTH, int GENERATION, Technology TEC , Mix MIX, boolean ALIVE, boolean ENTERED, boolean DIVERSIFIED,boolean ADOPTED,
		 boolean DAUGHTER,boolean MOTHER,double INITBUD,double DEBT,double BUD,double MUP,double RDEXPENDITURE) {
		id = ID;
		birth = BIRTH;
		generation = GENERATION;
		tec = TEC;
		mix = MIX;
		alive = ALIVE;
		entered	= ENTERED;
		diversified = DIVERSIFIED;
		adopted = ADOPTED;
		daughter = DAUGHTER;
		mother = MOTHER;
		initBud = INITBUD;
		debt = DEBT;
		bud = BUD;
		mup = MUP;
		RDExpenditure = RDEXPENDITURE;
	}
	firm[i] = new Firm(i, 1, 1, TR, m, true, false, false, false, false, false, a, a, a, markUp, RDexpendituresOnProfits);


	// constructor used by diversification
	Firm(int ID,int BIRTH, int GENERATION, Technology TEC , Mix MIX, boolean ALIVE, boolean ENTERED, boolean DIVERSIFIED,boolean ADOPTED,
		 boolean DAUGHTER,boolean MOTHER,double INITBUD,double DEBT,double BUD,double MUP,double RDEXPENDITURE, double AVERAGEEXPERIENCE, double EBW, double EXPERIENCE) {
		id = ID;
		birth = BIRTH;
		generation = GENERATION;
		tec = TEC;
		mix = MIX;
		alive = ALIVE;
		entered	= ENTERED;
		diversified = DIVERSIFIED;
		adopted = ADOPTED;
		daughter = DAUGHTER;
		mother = MOTHER;
		initBud = INITBUD;
		debt = DEBT;
		bud = BUD;
		mup = MUP;
		RDExpenditure = RDEXPENDITURE;
		experience = EXPERIENCE;
		averageExperience = AVERAGEEXPERIENCE;
		ebw = EBW;
	}

	double distanceFromCorner() { // returns (1 - the relative distance from the corner) with respect to the diagonal
		return 1 - Math.sqrt((this.tec.cheapLim-this.x.cheap)*(this.tec.cheapLim-this.x.cheap)+(this.tec.perfLim-this.x.perf)*(this.tec.perfLim-this.x.perf))/this.tec.diagonal;
	}							// return 1 if the firm is in the corner

	double distanceCovered() { // returns the fraction of the diagonal (from the origin to the tech corner) covered by the firm
		return Math.sqrt(this.x.cheap*this.x.cheap+this.x.perf*this.x.perf)/this.tec.diagonal;
	}
}

class Standard {
	double share;
	String label;
	int numberOfFirms;
}


class Product {
	double cheap; // design of product in terms of cheapness and performance
	double perf;
}

class UserClass {
	String label; // name of the userclass used in text output
	double cheapThreshold; // threshold of the user class
	double cheapExp; // exponent on cheapness (interest for)
	double perfThreshold;
	double perfExp;
	double brandLoyalty;
	double lambda; // common exponent to perf and cheap
	double alfaDesign; // sensitivity to design. It grows with the age of the market: from 1 to maxAlfaDesign linearly
	// variables
	int whenMktOpen; // period of first entry in the market
	int numOfFirstGenerationSellingFirms;
	int numOfFirstGenerationDiversifiedSellingFirms;
	int numOfSecondGenerationSellingFirms;
	int numOfDaughterSellingFirms;
	int generationOfLeader; // 1 for first gen 2 for second gen 3 for daughter
	double quantityPurchased;
	double herfindal;
	double profitRate;
	double maxProfitRate;
	double profitRateOfFirstGenerationLeader;  // leadership is share leadership
	double profitRateOfSecondGenerationLeader;
	double profitRateOfDaughterLeader;
	double shareOfFirstGenerationLeader;
	double shareOfSecondGenerationLeader;
	double shareOfDaughterLeader;
	double distanceOfFirstGenerationLeader;	 // leadership is technological leadership
	double distanceOfSecondGenerationLeader;
	double distanceOfDaughterLeader;
	int numOfSubMarkets;
	double techlevelCheap;
	double techlevelPerf;
	double MPshare;
	double TRshare;
	double DIVshare;
}

class Technology {
	double diagonal;
	String label;
	double cheapLim;
	double perfLim;
}

class ResTeam {
	int cheapEng;
	int perfEng;
	double averageExperience;
}

class Mix {
	double cheapMix;
	double perfMix;
	Mix randomMix() {
		double i;
		i=(Math.random()*101);
		if (i>100) i = 100;
		this.cheapMix = i/100;
		this.perfMix = (100-i)/100;
		return this;
	}
}



class NW {

		static DataOutputStream fp;

		static int NN; // NN is the number of firms	processed by the program
						   //
		static int
			t, // time index
			breakdown, // max computer life
			firstGeneration, // number of firms of the first generation at t=0
			secondGeneration,
			secondGenerationTime, // birth period of secon generation firms
			mpIntroduction, // period of MP discontinuity
			projectDuration, //	duration of project time
			fullVisible; // number of firms that makes all sbmkts present

		static Firm[] firm = new Firm[100];

		static double
			//parameters of the standard set relevant in calculating gammaDx
				gammaDxCheapSt = 0.0002,
				gammaDxPerfSt = 0.0002,
				alfa1DxSt = 1, // exponents of design change fn
				alfa2DxSt = 0.65,
				alfa3DxSt = 0.5,
				alfa4DxSt = 0,
				engineerCostSt = 0.001,
				projectDurationSt = 35,
				maxInitBudSt = 9,
				minInitBudSt = 9,
				TRPerfLimSt = 8000,
				TRCheapLimSt = 2000,


			r, // one period interest rate
			markUp,
			RDexpendituresOnProfits,
			minInitBud,	// range of random initial budget
			maxInitBud,
			initBudReductionFactor, // reduction factor for second generation firm
			fractionOfProfitForDebtPayBack,
			slopeOfFailure, wheightOfPast, // firm fails for negative trends (see bookkeeping)
 			gammaUt, // scale of utility fn
			lambdaUt,// other exponent of utility fn
			engineerCost, // cost of a single engineer
			gammaDxCheap, // scale of design change fn
			gammaDxPerf,
			alfa1Dx, alfa2Dx, alfa3Dx, alfa4Dx, // exponents of design change fn
			startAlfaDesign, // alfaDesign is an exponent on utility in demand probability function
			maxAlfaDesign, // it grows with the age of the market from startAlfaDesign to maxAlfaDesign
			bw,  // band wagon effect
			ebwErosion, // erosion of endogenous bw effect
			ebwExpenditure, // fraction of profits spent in ebw
			maxEbwExpenditure, // max useful level of ebw stock expenditure. Auto value is maxInitBud/10
			maxEbwEffect,
			errorDx,  // noise factor in design change
			errorDemand, // noise in sub markets sharing with respect to utility
			adoption1, // exponent on tec distance of the firm in adoption probability
			adoption2, // exponent on tec distance of  best the MP firm
			difficultyOfAdoption, // general exponent that decreases probability of adoption
			Efactor, // effectiveness of the average experience bonus
			percCostOfAdoption,	// cost of adoption in terms of budget fraction
			fixedCostOfAdoption, // fixed cost of adoption
			goDiversification, // when PC market becomes visible in terms of relative dimension (number of computers sold)
			//fixedCostOfDiversification, // diversification is possible if the mother has bud more than double of this cost
			percCostOfDiversification, // cost of diversification in terms of budget fraction
			fixedCostOfDiversification, // fixed cost
			percEbwDiversification; // percentual of ebw inherited by daughter

		static Technology TR = new Technology(); // parameters of technological frontiers
		static Technology MP = new Technology();
		static UserClass bfirm = new UserClass(); // parameters of user utility profile
		static UserClass indiv = new UserClass();
		static Standard Apple = new Standard();
		static Standard IBM = new Standard();


// ==========================================================================
// ==========================================================================
// ==========================================================================

		static boolean 	//switches
			multi = true,
			writeAllOnDisk = true,	   // it works for multi = true only and endOfMulti<2
			jump = true,
			randomJump = false,  // random choose of mix and design of an existing	PC firm. It works for jump = true only
			standard = false;

		static int
			end = 150, // number of periods in a single simulation
			endOfMulti = 5; // number of single simulations in average history

		public static void paramDefaultValues() {


			if ((writeAllOnDisk)&&(multi)&&(endOfMulti>2)) writeAllOnDisk = false;
			firstGeneration = 6; // number of firms of the first generation at t=0
			secondGeneration = 20;
			secondGenerationTime = 30; // birth period of secon generation firms
			mpIntroduction = 30; // period of MP discontinuity
			projectDuration = 35; //	duration of project time
			breakdown=12; // max computer life
			fullVisible = 5; // number of firms that makes all sbmkts present
			r = 0.025; // one period interest rate
			markUp = 0.1;
			RDexpendituresOnProfits = 0.1;
			minInitBud = 9;	// range of random initial budget
			maxInitBud = 9	;
			initBudReductionFactor = 0.45; // reduction factor for second generation firm
			fractionOfProfitForDebtPayBack = 0.15;
			slopeOfFailure = -0.03; // if profitRate decerases more than slopeOfFailure firm fails (if wheightOfPast = 1)
			wheightOfPast = 4; // it must be >= 1
			gammaUt = 0.08; // scale of utility fn
			engineerCost = 0.001; // cost of a single engineer

			// gammaDx is calculated at the end of this procedure
			alfa1Dx = 1; // exponents of design change fn
			alfa2Dx = 0.95;
			alfa3Dx = 0.25;
			alfa4Dx = 0;
			startAlfaDesign = 2;
			maxAlfaDesign = 2;
			bw = 1;  // band wagon effect
			ebwErosion = 0.1; // erosion of endogenous bw effect
			ebwExpenditure = 0.1; // fraction of profits spent in ebw
			maxEbwExpenditure = maxInitBud/10; // max useful level of ebw stock expenditure. Auto value is maxInitBud/10
			maxEbwEffect = 1.5; // maxx ebw possible effect
			errorDx = 0.05;  // +- noise factor in design change
			errorDemand = 0.1; // noise in sub markets sharing with respect to utility
			adoption1 = 1; // exponent on tec distance of the firm in adoption probability
			adoption2 = 1; // exponent on tec distance of  best the MP firm

			difficultyOfAdoption =1; // general exponent that decreases probability of adoption
			Efactor = 1; // effectiveness of the average experience bonus
			percCostOfAdoption = 0.5; // cost of adoption in terms of budget fraction
			fixedCostOfAdoption = 1; // fixed cost of adoption
			goDiversification = 0.4; // when PC market becomes visible in terms of relative dimension with respect to mainframe mkt (number of computers sold)
			fixedCostOfDiversification = 0; // diversification is possible if the mother has bud more than double of this cost
			percCostOfDiversification = 0.5; // perc cost on budget for diversification
			percEbwDiversification = 0.8; // percentual of ebw inherited by daughter


			// parameters of technological frontiers
							TR.label = "TR";
							TR.cheapLim = 2000;
							TR.perfLim = 8000;
							TR.diagonal = Math.sqrt(TR.cheapLim*TR.cheapLim+TR.perfLim*TR.perfLim);
							MP.label = "MP";
							MP.cheapLim = 9000;
							MP.perfLim = 9000;
							MP.diagonal = Math.sqrt(MP.cheapLim*MP.cheapLim+MP.perfLim*MP.perfLim);
			// parameters of user utility profile
							bfirm.label = "BFIRM";
							bfirm.cheapExp = 0.2;
							bfirm.cheapThreshold = 400;
							bfirm.perfExp = 0.8;
							bfirm.perfThreshold = 4000;
							bfirm.lambda = 0;
							bfirm.alfaDesign = startAlfaDesign;
							bfirm.whenMktOpen = 0;
							bfirm.brandLoyalty = 0;
							bfirm.quantityPurchased = 0;
							bfirm.numOfSubMarkets = 3000;

							indiv.label = "INDIV";
							indiv.cheapExp = 0.8;
							indiv.cheapThreshold = 2000;
							indiv.perfExp = 0.2;
							indiv.perfThreshold = 500;
							indiv.lambda = 0.05;
							indiv.alfaDesign = startAlfaDesign;
							indiv.whenMktOpen = 0;
							indiv.brandLoyalty = 0;
							indiv.quantityPurchased = 0;
							indiv.numOfSubMarkets = 3000;
			// standards
							Apple.label = "Apple";
							IBM.label = "IBM  ";

			//*************************************************************8
			// calculates gammaDx for the two dimensions
			double RR0, EE0, RR1, EE1;
			RR0 = (0.5*(minInitBudSt+maxInitBudSt))/(2*engineerCostSt*projectDurationSt);
			EE0 = projectDurationSt;
			RR1 = (0.5*(minInitBud+maxInitBud))/(2*engineerCost*projectDuration);
			EE1 = projectDuration;
			gammaDxCheap =	gammaDxCheapSt *
							Math.pow(0.5*TRCheapLimSt,alfa1DxSt)/Math.pow(0.5*TR.cheapLim,alfa1Dx) *
							Math.pow(RR0,alfa2DxSt)/Math.pow(RR1,alfa2Dx) *
							Math.pow(EE0,alfa3DxSt)/Math.pow(EE1,alfa3Dx) *
							TR.cheapLim/TRCheapLimSt;
			gammaDxPerf =	gammaDxPerfSt *
							Math.pow(0.5*TRPerfLimSt,alfa1DxSt)/Math.pow(0.5*TR.perfLim,alfa1Dx) *
							Math.pow(RR0,alfa2DxSt)/Math.pow(RR1,alfa2Dx) *
							Math.pow(EE0,alfa3DxSt)/Math.pow(EE1,alfa3Dx) *
							TR.perfLim/TRPerfLimSt;


	}


	public static void printParamsOnDisk() {

		printMulti("NAME OF PARAMETER SET: exp T-, exp R+, Apc+ (+1) easy adoption \n");

		printMulti("\n\nSWITCHES");
		printMulti("\nStandards in PC market? "+standard);
		printMulti("\nJump diversification? "+jump+"    randomJump?"+ randomJump);

		printMulti("\n\nPARAMETERS");
		printMulti("\nNumber of first generation firms "+firstGeneration);
		printMulti("\nNumber of second generation firms "+secondGeneration);
		printMulti("\nMicroprocessor introduction "+mpIntroduction);
		printMulti("\nProject duration "+projectDuration);
		printMulti("\nNumber of big firm   submarkets "+bfirm.numOfSubMarkets);
		printMulti("\nNumber of individual submarkets "+indiv.numOfSubMarkets);
		printMulti("\nMax computer age "+breakdown);
		printMulti("\nMin initial budget "+minInitBud);
		printMulti("\nMax initial budget "+maxInitBud);
		printMulti("\nInit bud reduction for second generation "+initBudReductionFactor);
		printMulti("\nFraction of profits for debt payback "+fractionOfProfitForDebtPayBack);
		printMulti("\nInterest rate "+r);
		printMulti("\nMarkup "+markUp);
		printMulti("\nR&D expenditures on profits "+RDexpendituresOnProfits);
		printMulti("\nEngineer cost "+engineerCost);
		printMulti("\nUTILITY");
		printMulti("\n      BIG FIRM");
		printMulti("\n         cheap threshold "+bfirm.cheapThreshold);
		printMulti("\n         perf  threshold "+bfirm.perfThreshold);
		printMulti("\n         exponent on cheapness "+bfirm.cheapExp);
		printMulti("\n         exponent on performance "+bfirm.perfExp);
		printMulti("\n         exponent lambda "+bfirm.lambda);
		printMulti("\n      INDIV");
		printMulti("\n         cheap threshold "+indiv.cheapThreshold);
		printMulti("\n         perf  threshold "+indiv.perfThreshold);
		printMulti("\n         exponent on cheapness "+indiv.cheapExp);
		printMulti("\n         exponent on performance "+indiv.perfExp);
		printMulti("\n         exponent lambda "+indiv.lambda);
		printMulti("\n      COMMON PARAMETERS");
		printMulti("\n         scale parameter "+gammaUt);
		printMulti("\nDEMAND");
		printMulti("\n      Design sensitivity dynamics "+startAlfaDesign+" -> "+maxAlfaDesign+" (PC = 3)");
		printMulti("\n      Exponent on mkt. share "+bw+"PC=0.1");
		printMulti("\n      Advertising expenditures on profits "+ebwExpenditure);
		printMulti("\n      Advertising erosion "+ebwErosion);
		printMulti("\n      Exponent on advertising "+maxEbwEffect);
		printMulti("\n      BL (brand loyalty): BIGFIRM "+bfirm.brandLoyalty+" INDIV "+indiv.brandLoyalty);
		printMulti("\nFEASIBILITY FRONTIERS");
		printMulti("\n      TR: cheap "+TR.cheapLim+" perf "+TR.perfLim);
		printMulti("\n      MP: cheap "+MP.cheapLim+" perf "+MP.perfLim);
		printMulti("\nDESIGN CHANGE");
		printMulti("\n      scale parameter (cheapDx/perfDx) "+gammaDxCheap+"/"+gammaDxPerf);
		printMulti("\n      exponent on (L-X) "+alfa1Dx);
		printMulti("\n      exponent on  R    "+alfa2Dx);
		printMulti("\n      exponent on  T    "+alfa3Dx);
		printMulti("\n      exponent on  AE   "+alfa4Dx);
		printMulti("\nADOPTION");
		printMulti("\n      adoption difficulty (it decreases the adoption Prob) "+difficultyOfAdoption);
		printMulti("\n      fixed cost "+fixedCostOfAdoption);
		printMulti("\n      perc cost on budget "+percCostOfAdoption);
		printMulti("\nDIVERSIFICATION");
		printMulti("\n      relative size (num of computers) of PC market "+goDiversification);
		printMulti("\n      fixed cost "+fixedCostOfDiversification);
		printMulti("\n      perc cost on budget "+percCostOfDiversification);
		printMulti("\n      Advertising inheritance "+percEbwDiversification);
		printMulti("\n\n\n\n\n");
	}

	public static String timer() {
		String str;
		Date h = new Date();
		str = String.valueOf(h.getMinutes())+"."+String.valueOf(h.getSeconds());
		return str;
	}


	public static double  S(double x, double maxX, double maxY) {

		x=x/maxX;
			if (x<=0) return 0;
				else if (x<=0.25) return (x*0.5)*maxY;
					else if ((x>0.25)&&(x<=0.75)) return ((x-0.25)*1.5+0.125)*maxY;
						else if ((x>0.75)&&(x<=1)) return ((x-0.75)*0.5+0.875)*maxY;
							else if (x>1) return maxY;
			return 1;

   /* logistic function: this function is defined in the box [0,0][1,1]
      on x values 0, 0.25, 0.5, 0.75, 1. The input value is parametrized
      on maxX and the output value on maxY: if X>maxX then y=maxY whilest
      we have a 's' shaped relationship for other values of x.            */
	}


	public static int roundInt(double a) {
		if (a-(int)a>=0.5) return (int)(a+1);
		else return (int)a;
	}


	public static int randomInt(int a) {   // integer uniform distribution between [0, a)
		int w;
		w = (int)(Math.random()*a);
		if (w == a) w--;
		return w;
	}

	/*public static double kbdReadDouble() {
		DataInput kbd = new DataInputStream(System.in);
		Double a;
		String str;
		try {
			str=kbd.readLine();
		} catch (IOException ignored) {}
		try {
			a = Double.valueOf(str);
		} catch (NumberFormatException e) {}
		return a.doubleValue();
	}

	public static int kbdReadInt() {
		DataInput kbd = new DataInputStream(System.in);
		Integer a;
		String str;
		try {
			str=kbd.readLine();
		} catch (IOException ignored) {}
		try {
			a = Integer.valueOf(str);
		} catch (NumberFormatException e) {}
		return a.intValue();
	}


	public static String kbdReadString() {
		DataInput kbd = new DataInputStream(System.in);
		String str;
		try {
			str=kbd.readLine();
		} catch (IOException ignored) {}
		return str;
	}*/


	public static String f(double k,int n) { //format double numbers in n length string
		String str;
		str = String.valueOf(k);
		return str.concat("          ").substring(0,n);
	}


	public static String f(int k,int n) { //format int numbers in n length string
		String str;
		str = String.valueOf(k);
		return str.concat("          ").substring(0,n);
	}



	public static double utility(Firm f, UserClass uc) {

		double marketCheap;
		double u;
		marketCheap = f.x.cheap/(1+f.mup);
		if ((marketCheap<=uc.cheapThreshold)||(f.x.perf<=uc.perfThreshold)) return 0;
		u = gammaUt * Math.pow((marketCheap-uc.cheapThreshold),(uc.lambda+uc.cheapExp)) * Math.pow((f.x.perf-uc.perfThreshold),(uc.lambda+uc.perfExp));
		return u;
	}


	public static void RDInvest() { // Manages research team of engineers.
									// Calculates avg experience of team if
									// the team grows. Generally
									// innovation expenditures are on profits
									// but if project time has not finished then
									// expenditures are on a constant fraction
									// of initial budget (+ profits' contribution if there are).
		int i, h, k;
		print("\nTECHNOLOGY REPORT");
		for(i=1;i<=NN;i++) if (firm[i].alive) {
			k = firm[i].res.cheapEng+firm[i].res.perfEng;
			if (t-firm[i].birth<projectDuration) { //  firm hasn't finished its project yet
				firm[i].res.cheapEng = (int)(((firm[i].initBud/projectDuration + firm[i].profit*firm[i].RDExpenditure)*firm[i].mix.cheapMix)/engineerCost);
				firm[i].res.perfEng  = (int)(((firm[i].initBud/projectDuration + firm[i].profit*firm[i].RDExpenditure)*firm[i].mix.perfMix)/engineerCost);
				firm[i].bud -= firm[i].initBud/projectDuration + firm[i].profit*firm[i].RDExpenditure;
			}
			else { // initial project has finished and debt payback is on
				if ((firm[i].profit*firm[i].RDExpenditure)<k*engineerCost) { // profits are too low to mantain current team
					firm[i].res.cheapEng = (int) (firm[i].res.cheapEng*0.9);
					firm[i].res.perfEng  = (int) (firm[i].res.perfEng*0.9);
					firm[i].bud -= (int)(k*0.9)*engineerCost;
				}
				else { // profits are enough to mantain current	team at least
					firm[i].res.cheapEng = (int)(((firm[i].profit*firm[i].RDExpenditure)*firm[i].mix.cheapMix)/engineerCost);
					firm[i].res.perfEng  = (int)(((firm[i].profit*firm[i].RDExpenditure)*firm[i].mix.perfMix)/engineerCost);
					firm[i].bud -= firm[i].profit*firm[i].RDExpenditure;
				}
			}
			h = firm[i].res.cheapEng+firm[i].res.perfEng;
			if ((firm[i].bud<=0)||(h<1)) failFirm(i); // kills insignificant budget firms
			firm[i].averageExperience ++;
			if (h>k) firm[i].averageExperience = firm[i].averageExperience * (float)k/h + (float)(h-k)/h;
			showFirmTechnology(i);
		}
	}



	public static void ebwInvest() {

		int i;
		for(i=1;i<=NN;i++) if (firm[i].alive) {
			firm[i].ebw = firm[i].ebw * (1-ebwErosion) + ebwExpenditure * firm[i].profit;
			firm[i].bud -= firm[i].profit*ebwExpenditure;
			if (firm[i].bud<=0) firm[i].alive=false; // kill firm
		}
	}



	public static double findBestMPDistance() {

		int i;
		double n;
		double m = 0;
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].tec.label=="MP")) {
			n = firm[i].distanceCovered();
			if (n>m) m = n;
		}
		return m;
	}


XXXX
	public static void adoption() {

		int i;
		double m, probability, e;
		m = findBestMPDistance();
		print("\nADOPTION REPORT: ");
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].entered)&&(firm[i].tec==TR)) {
			probability = Math.pow( 0.5 * Math.pow(firm[i].distanceCovered(),adoption1) + 0.5 * Math.pow(m,adoption2) , difficultyOfAdoption);
			print("F"+i+" Pr"+probability+" "+firm[i].distanceCovered()+" "+m);
			if ( (Math.random()<probability) && (firm[i].bud*(1-percCostOfAdoption)-fixedCostOfAdoption>0) ) {
				print("*");
				firm[i].bud = firm[i].bud*(1-percCostOfAdoption)-fixedCostOfAdoption;
				firm[i].tec = MP;
				firm[i].adopted = true;
				e = firm[i].experience;
				firm[i].experience  = 1 + roundInt(Efactor * (e-firm[i].averageExperience));
				// bonus if average experience is low
				if (firm[i].experience>e) firm[i].experience = e;
				firm[i].averageExperience = 1;
			}
		}
		print("\n");
	}


	public static int findWorstPCFirm() {
		int i, k;
		for (k=1;k<=NN;k++)	if ((firm[k].alive)&&(firm[k].servedMkt==2)) break;
		// find the first index of PC firm in order to begin comparisons
		for (i=k+1;i<=NN;i++) if ((firm[k].alive)&&(firm[k].servedMkt==2)) {
			if (firm[i].distanceCovered()>firm[k].distanceCovered()) k=i;
		}
		return k;
	}

  public static double findMedCheapPCFirm(){
        int i, k, ii, kk=0, ll;
        double oo1=0,pp1=0;
        for(k=1;k<=NN;k++) if((firm[k].alive)&&(firm[k].servedMkt==2)) {
            oo1+=firm[k].x.cheap;
            kk++;
        }
        pp1=oo1/kk;
        return pp1;
    }
     public static double findMedPerfPCFirm(){
        int i, k, ii, kk=0, ll;
        double oo2=0,pp2=0;
        for(k=1;k<=NN;k++) if((firm[k].alive)&&(firm[k].servedMkt==2)) {
            oo2+=firm[k].x.perf;
            kk++;
        }
        pp2=oo2/kk;
        return pp2;
     }





	public static int findOnePCFirm() {
		int i;
		int[] zz = new int[NN];
		for (i=firstGeneration+1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].servedMkt==2)) zz[++zz[0]]=i;
		if (zz[0]>0) return zz[1+randomInt(zz[0])];
		else return 0;
	}


	public static void diversification() {
		int i, k = 0;
		for (i=1;i<=NN;i++) if ((firm[i].alive)&&(!firm[i].mother)&&(!firm[i].daughter)&&(firm[i].servedMkt==1)&&(firm[i].tec.label=="MP")&&(firm[i].profitRate>0)&&(firm[i].bud*(1-percCostOfDiversification)>fixedCostOfDiversification)) {
			Mix m = new Mix();
			if (jump) {
				NN++;
				m.randomMix();
				firm[NN] = new Firm(NN, t, 3, MP, m, true, false, false, false, true, false, firm[i].bud*percCostOfDiversification, firm[i].bud*percCostOfDiversification, firm[i].bud*percCostOfDiversification, markUp, RDexpendituresOnProfits, firm[i].averageExperience, firm[i].ebw*percEbwDiversification, (firm[i].experience - firm[i].averageExperience) * Efactor);
				if (!randomJump) {
					//firm[NN].x.cheap = firm[findWorstPCFirm()].x.cheap;
					//firm[NN].x.perf  = firm[findWorstPCFirm()].x.perf;
					firm[NN].x.cheap=findMedCheapPCFirm();
					firm[NN].x.perf = findMedPerfPCFirm();

				}
				else {  // random diversification: design and mix picked up from an existing second generation firm
					k = findOnePCFirm();
					firm[NN].x.cheap = firm[k].x.cheap;
					firm[NN].x.perf = firm[k].x.perf;
					firm[NN].mix.cheapMix = firm[k].mix.cheapMix;
					firm[NN].mix.perfMix = firm[k].mix.perfMix;
				}
				firm[i].bud = firm[i].bud*(1-percCostOfDiversification) - fixedCostOfDiversification;
				firm[NN].diversified = true;
				firm[NN].entered = true;	 // the daughter is set to "entered" here and not by checkEntry() procedure
				firm[NN].whenEntered = t; // the problem is that daughters begin from a design good for both indiv and bfirm markets
				firm[NN].servedUserClass = indiv;
				firm[NN].servedMkt = 2;
				if (standard) {
					findStandardShares();
					// first diversifing firm is IBM
					if (NN == firstGeneration + secondGeneration + 1) firm[NN].stan = IBM;
					else {
						if (Math.random()<IBM.share) firm[NN].stan = IBM;
						else firm[NN].stan = Apple;
					}
				}
				// be carefull: the firm constructor is integrated by lines that modify some values, e.g. "entered" is false in constructor and set true imediately after
				print("\nDIVERSIFICATION: firm "+i+" generates firm "+NN);
				firm[i].mother = true;
			}
			else if (firm[i].x.cheap/(1+firm[i].mup)>=indiv.cheapThreshold) { // mainf firm must be in PC area
				NN++;
				m = firm[i].mix;
				firm[NN] = new Firm(NN, t, 3, MP, m, true, false, false, false, true, false, firm[i].bud*percCostOfDiversification, firm[i].bud*percCostOfDiversification, firm[i].bud*percCostOfDiversification, markUp, RDexpendituresOnProfits, firm[i].averageExperience, firm[i].ebw*percEbwDiversification, (firm[i].experience - firm[i].averageExperience) * Efactor);
				firm[NN].x.cheap = firm[i].x.cheap;
				firm[NN].x.perf  = firm[i].x.perf;
				firm[i].bud = firm[i].bud*(1-percCostOfDiversification) - fixedCostOfDiversification;
				firm[NN].diversified = true;
				firm[NN].entered = true;	 // the daughter is set to "entered" here and not by checkEntry() procedure
				firm[NN].whenEntered = t; // the problem is that daughters begin from a design good for both indiv and bfirm markets
				firm[NN].servedUserClass = indiv;
				firm[NN].servedMkt = 2;
				if (standard) {
					findStandardShares();
					// first diversifing firm is IBM
					if (NN == firstGeneration + secondGeneration + 1) firm[NN].stan = IBM;
					else {
						if (Math.random()<IBM.share) firm[NN].stan = IBM;
						else firm[NN].stan = Apple;
					}
				}
				// be carefull: the firm constructor is integrated by lines that modify some values, e.g. "entered" is false in constructor and set true imediately after
				print("\nDIVERSIFICATION: firm "+i+" generates firm "+NN+" cheap "+firm[i].x.cheap);
				if (randomJump) print(" imitation of firm "+k);
				firm[i].mother = true;
			}
		}
	}


	public static void innovation() {

		int i;
		for(i=1;i<=NN;i++) if (firm[i].alive) {
			firm[i].x.cheap += gammaDxCheap
				* Math.pow((firm[i].tec.cheapLim - firm[i].x.cheap),alfa1Dx)
				* Math.pow(firm[i].res.cheapEng,alfa2Dx)
				* Math.pow(firm[i].experience,alfa3Dx)
				* Math.pow(firm[i].averageExperience,alfa4Dx);
			if (firm[i].x.cheap>firm[i].tec.cheapLim) firm[i].x.cheap=firm[i].tec.cheapLim;
			firm[i].x.perf += gammaDxPerf
				* Math.pow((firm[i].tec.perfLim - firm[i].x.perf),alfa1Dx)
				* Math.pow(firm[i].res.perfEng,alfa2Dx)
				* Math.pow(firm[i].experience,alfa3Dx)
				* Math.pow(firm[i].averageExperience,alfa4Dx);
			if (firm[i].x.perf>firm[i].tec.perfLim) firm[i].x.perf=firm[i].tec.perfLim;
			firm[i].experience ++;
		}
	}


	public static void checkEntry() {

		int i;
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(!firm[i].entered)) {
			if (utility(firm[i],bfirm)>0) {
				firm[i].entered = true;
				if (bfirm.whenMktOpen == 0) bfirm.whenMktOpen = t;
				firm[i].whenEntered = t;
				firm[i].servedUserClass = bfirm;
				firm[i].servedMkt = 1;
				showFirmCheckEntry(i);
			}
			if (utility(firm[i],indiv)>0) {
				firm[i].entered = true;
				if (indiv.whenMktOpen == 0) indiv.whenMktOpen = t;
				firm[i].whenEntered = t;
				firm[i].servedUserClass = indiv;
				firm[i].servedMkt = 2;
				if (standard) {
					// first entering firm is Apple
					if ((IBM.share == 0)&&(Apple.share == 0)) firm[i].stan = Apple;
					else {
						if (Math.random()<IBM.share) firm[i].stan = IBM;
						else firm[i].stan = Apple;
					}
				}
				showFirmCheckEntry(i);
			}
		}
	}


	public static void print(String str) {
		if ((writeAllOnDisk)||(!multi)) {
			//System.out.println(str);
			try {fp.writeBytes(str+"\n");} catch (IOException e) {System.out.println(e.getMessage());}
		}
	}


	public static void printMulti(String str) {
			try {fp.writeBytes(str);} catch (IOException e) {System.out.println(e.getMessage());}
	}


/*	public static void showFirmMarket(int i) {
		print("F"+f(i,2)+" we"+f(firm[i].whenEntered,3)+" sh"+f(firm[i].share,5)+" ebw"+f(firm[i].ebw,4)+" pf"+f(firm[i].profit,5));
	}
*/

	public static void showFirmMarket(int i) {
		print("F"+f(i,2)+" "+firm[i].tec.label+" "+" birth "+firm[i].birth+" we"+f(firm[i].whenEntered,3)+" sh"+f(firm[i].share,5)+" ebw"+f(firm[i].ebw,4)+" pf"+f(firm[i].profit,5)+" std"+firm[i].stan.label+" M"+firm[i].numberOfServedSubMarkets+" B"+firm[i].numberOfBreakdowns+" N"+firm[i].numberOfNewSubMarkets);
	}


	public static void showFirmTechnology(int i) {
		print("F"+f(i,2)+" "+firm[i].tec.label+"  R("+f(firm[i].res.cheapEng,3)+","+f(firm[i].res.perfEng,3)+")"+" AE"+f(firm[i].averageExperience,4)+"  mix("+f(roundInt(100*firm[i].mix.cheapMix),2)+","+f(roundInt(100*firm[i].mix.perfMix),2)+")"+"  X("+f((int)firm[i].x.cheap,5)+","+f((int)firm[i].x.perf,5)+")"+" b"+f(firm[i].bud,5)+" dist "+firm[i].distanceFromCorner());
	}


	public static void showFirmBookeeping(int i) {
		print("firm "+i+" b"+f(firm[i].bud,5)+" d"+f(firm[i].debt,5)+" rate"+f(firm[i].profitRate,5)+" slopeOfRate"+f(firm[i].slopeOfProfitRate,5)+" M"+firm[i].numberOfServedSubMarkets+" B"+firm[i].numberOfBreakdowns+" N"+firm[i].numberOfNewSubMarkets);
	}

	public static void showFirmCheckEntry(int i) {
		print("\nNEW MARKET ENTRY: firm "+i+"\n");
	}




	public static void marketBfirm() {

		int i;
		int j = 0, h = 0;
		double cumulatedProb = 0;
		bfirm.techlevelCheap=0;
		bfirm.techlevelPerf =0;
		bfirm.TRshare=0;
		bfirm.MPshare=0;
		int numOfSellingFirms = 0;
		int numberOfPurchasingSubMarkets;
		int busySubMarkets = 0; // cumulates sub markets not free
		if (bfirm.whenMktOpen > 0) bfirm.alfaDesign += (maxAlfaDesign - startAlfaDesign)/(end-bfirm.whenMktOpen);
		bfirm.quantityPurchased = 0;
		bfirm.herfindal = 0;
		bfirm.numOfFirstGenerationSellingFirms = 0;
		bfirm.numOfFirstGenerationDiversifiedSellingFirms = 0;
		bfirm.numOfSecondGenerationSellingFirms = 0;
		bfirm.numOfDaughterSellingFirms = 0;
		bfirm.generationOfLeader = 0;
		bfirm.shareOfFirstGenerationLeader = 0;
		bfirm.profitRateOfFirstGenerationLeader = 0;
		bfirm.distanceOfFirstGenerationLeader = 0;
		double shareOfLeader = 0;
		double shareOfFirstGenerationLeader = 0;
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].servedMkt==1)) {
			numOfSellingFirms ++;
			if (firm[i].generation == 1) {
				bfirm.numOfFirstGenerationSellingFirms++;
				if (firm[i].adopted) bfirm.numOfFirstGenerationDiversifiedSellingFirms++;
			}
			if (firm[i].generation == 2) bfirm.numOfSecondGenerationSellingFirms++;
			if (firm[i].generation == 3) bfirm.numOfDaughterSellingFirms++;
			firm[i].numberOfNewSubMarkets = 0;
			firm[i].u = utility(firm[i], bfirm);
			firm[i].share = Math.pow(firm[i].u,bfirm.alfaDesign) * Math.pow(1+firm[i].share,bw )*Math.pow(1+S(firm[i].ebw,maxEbwExpenditure,maxEbwEffect),maxEbwEffect);
			// firm[i].share is used here to store a probability
			firm[i].share *= 1-errorDemand+2*Math.random()*errorDemand;
			cumulatedProb += firm[i].share;
			if (firm[i].numberOfServedSubMarkets>0) {
				firm[i].numberOfBreakdowns = (int)(firm[i].numberOfServedSubMarkets*firm[i].averageAgeOfSubMarkets/breakdown);
				firm[i].numberOfBLReturns = (int)(firm[i].numberOfBreakdowns * bfirm.brandLoyalty);
				firm[i].numberOfServedSubMarkets -= firm[i].numberOfBreakdowns;
				firm[i].numberOfNewSubMarkets = firm[i].numberOfBLReturns;
			}
			busySubMarkets += firm[i].numberOfServedSubMarkets + firm[i].numberOfBLReturns;
		}
		if (numOfSellingFirms <= fullVisible) numberOfPurchasingSubMarkets = (int)((bfirm.numOfSubMarkets-busySubMarkets)*(double)numOfSellingFirms/fullVisible);
		else numberOfPurchasingSubMarkets = bfirm.numOfSubMarkets-busySubMarkets;
		print("\nBFIRM MARKET: busy sub mkts "+busySubMarkets+" purchasing SM "+numberOfPurchasingSubMarkets+ " DS="+bfirm.alfaDesign);
		// if there are few firms in the market then  purchasing sub market are less than free sub markets
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].servedMkt==1)) {
			firm[i].share = firm[i].share/cumulatedProb;
			// firm[i].share becomes a real share
			if (shareOfLeader<firm[i].share) {	 // find the generation of the leader
				shareOfLeader = firm[i].share;
				bfirm.generationOfLeader = firm[i].generation;
			}
			if ((firm[i].generation == 1)&&(shareOfFirstGenerationLeader<firm[i].share)) {
				shareOfFirstGenerationLeader = firm[i].share;
				bfirm.shareOfFirstGenerationLeader = shareOfFirstGenerationLeader;
				bfirm.profitRateOfFirstGenerationLeader = firm[i].profitRate;
				bfirm.distanceOfFirstGenerationLeader = firm[i].distanceFromCorner();
			}
			bfirm.herfindal += (firm[i].share * firm[i].share);
			h = roundInt(firm[i].share * (numberOfPurchasingSubMarkets));
			// h is the number of sub markets captured with competition
			firm[i].numberOfNewSubMarkets += h;
			bfirm.quantityPurchased += firm[i].numberOfNewSubMarkets * firm[i].u;
			// calculates the number of computer sold in mainframe market
			firm[i].averageAgeOfSubMarkets = 1 + firm[i].averageAgeOfSubMarkets*(double)(firm[i].numberOfServedSubMarkets)/(firm[i].numberOfServedSubMarkets+firm[i].numberOfNewSubMarkets);
			// average age of sub market set of the firm is calculated
			firm[i].numberOfServedSubMarkets += firm[i].numberOfNewSubMarkets;
			firm[i].profit = (1/firm[i].x.cheap)*firm[i].mup*firm[i].u*firm[i].numberOfNewSubMarkets;
			firm[i].bud += firm[i].profit;
			bfirm.techlevelCheap +=firm[i].share*firm[i].x.cheap;
			bfirm.techlevelPerf +=firm[i].share*firm[i].x.perf;
			showFirmMarket(i);
		}
		print("HERFINDAL "+bfirm.herfindal+ " COMPUTERS SOLD "+bfirm.quantityPurchased);
		for(i=1;i<=NN;i++){
		    if((firm[i].alive)&&(firm[i].servedMkt==1)&&(firm[i].generation==1)) bfirm.TRshare += firm[i].share;
		}
		for(i=1;i<=NN;i++){
		    if((firm[i].alive)&&(firm[i].servedMkt==1)&&(firm[i].generation==2)) bfirm.MPshare += firm[i].share;
		}
	}


	public static void findStandardShares() {
		int i;
		Apple.share = 0;
		IBM.share = 0;
		Apple.numberOfFirms = 0;
		IBM.numberOfFirms = 0;
		for (i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].servedMkt==2)) {
			if (firm[i].stan == IBM) {
				IBM.share += firm[i].share;
				IBM.numberOfFirms++;
			}
			else {
				Apple.share += firm[i].share;
				Apple.numberOfFirms++;
			}
		}
	}


	public static void marketIndiv() {

		int i;
		int j = 0, h = 0;
		double cumulatedProb = 0;
		indiv.techlevelCheap=0;
		indiv.techlevelPerf =0;
		indiv.MPshare=0;
		indiv.DIVshare=0;
		int numOfSellingFirms = 0;
		int numberOfPurchasingSubMarkets;
		int busySubMarkets = 0; // cumulates sub markets not free
		if (indiv.whenMktOpen > 0) indiv.alfaDesign += (maxAlfaDesign - startAlfaDesign)/(end-indiv.whenMktOpen);
		indiv.quantityPurchased = 0;
		indiv.herfindal = 0;
		indiv.numOfFirstGenerationSellingFirms = 0;
		indiv.numOfSecondGenerationSellingFirms = 0;
		indiv.numOfDaughterSellingFirms = 0;
		indiv.generationOfLeader = 0;
		indiv.shareOfSecondGenerationLeader = 0;
		indiv.shareOfDaughterLeader = 0;
		indiv.profitRateOfSecondGenerationLeader = 0;
		indiv.profitRateOfDaughterLeader = 0;
		indiv.distanceOfSecondGenerationLeader = 0;
		indiv.distanceOfDaughterLeader = 0;
		double shareOfLeader = 0;
		double shareOfSecondGenerationLeader = 0;
		double shareOfDaughterLeader = 0;
		if (standard) findStandardShares();
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].servedMkt==2)) {
			numOfSellingFirms ++;
			if (firm[i].generation == 1) indiv.numOfFirstGenerationSellingFirms++;
			if (firm[i].generation == 2) indiv.numOfSecondGenerationSellingFirms++;
			if (firm[i].generation == 3) indiv.numOfDaughterSellingFirms++;
			firm[i].numberOfNewSubMarkets = 0;
			firm[i].u = utility(firm[i], indiv);
			if (standard) firm[i].share = Math.pow(firm[i].u,indiv.alfaDesign) * Math.pow(1+firm[i].stan.share,bw+S(firm[i].ebw, maxEbwExpenditure, maxEbwEffect));
			else firm[i].share = Math.pow(firm[i].u,indiv.alfaDesign+1) * Math.pow(1+firm[i].share,bw-0.9)*Math.pow(1+S(firm[i].ebw,maxEbwExpenditure,maxEbwEffect),maxEbwEffect+1);
			// firm[i].share is used here to store a probability
			firm[i].share *= 1-errorDemand+2*Math.random()*errorDemand;
			cumulatedProb += firm[i].share;
			if (firm[i].numberOfServedSubMarkets>0) {
				firm[i].numberOfBreakdowns = (int)(firm[i].numberOfServedSubMarkets*firm[i].averageAgeOfSubMarkets/breakdown);
				firm[i].numberOfBLReturns = (int)(firm[i].numberOfBreakdowns * indiv.brandLoyalty);
				firm[i].numberOfServedSubMarkets -= firm[i].numberOfBreakdowns;
				firm[i].numberOfNewSubMarkets = firm[i].numberOfBLReturns;
			}
			busySubMarkets += firm[i].numberOfServedSubMarkets + firm[i].numberOfBLReturns;
		}
		if (numOfSellingFirms <= fullVisible) numberOfPurchasingSubMarkets = (int)((indiv.numOfSubMarkets-busySubMarkets)*(double)numOfSellingFirms/fullVisible);
		else numberOfPurchasingSubMarkets = bfirm.numOfSubMarkets-busySubMarkets;
		print("\nINDIV MARKET: busy sub mkts "+busySubMarkets+" purchasing SM "+numberOfPurchasingSubMarkets+ " DS="+indiv.alfaDesign);
		// if there are few firms in the market then  purchasing sub market are less than free sub markets
		for(i=1;i<=NN;i++) if ((firm[i].alive)&&(firm[i].servedMkt==2)) {
			firm[i].share = firm[i].share/cumulatedProb;
			// firm[i].share becomes a real share here
			if (shareOfLeader<firm[i].share) {	 // find the generation of the leader
				shareOfLeader = firm[i].share;
				indiv.generationOfLeader = firm[i].generation;
			}
			if ((firm[i].generation == 2)&&(shareOfSecondGenerationLeader<firm[i].share)) {
				shareOfSecondGenerationLeader = firm[i].share;
				indiv.shareOfSecondGenerationLeader = shareOfSecondGenerationLeader;
				indiv.profitRateOfSecondGenerationLeader = firm[i].profitRate;
				indiv.distanceOfSecondGenerationLeader = firm[i].distanceFromCorner();
			}
			if ((firm[i].generation == 3)&&(shareOfDaughterLeader<firm[i].share)) {
				shareOfDaughterLeader = firm[i].share;
				indiv.shareOfDaughterLeader = shareOfDaughterLeader;
				indiv.profitRateOfDaughterLeader = firm[i].profitRate;
				indiv.distanceOfDaughterLeader = firm[i].distanceFromCorner();
			}
			indiv.herfindal += (firm[i].share * firm[i].share);
			h = roundInt(firm[i].share*(numberOfPurchasingSubMarkets));
			// h is the number of sub markets captured with competition
			firm[i].numberOfNewSubMarkets += h;
			indiv.quantityPurchased += firm[i].numberOfNewSubMarkets * firm[i].u;
			firm[i].averageAgeOfSubMarkets = 1 + firm[i].averageAgeOfSubMarkets*(double)(firm[i].numberOfServedSubMarkets)/(firm[i].numberOfServedSubMarkets+firm[i].numberOfNewSubMarkets);
			// average age of sub market set of the firm is calculated
			firm[i].numberOfServedSubMarkets += firm[i].numberOfNewSubMarkets;
			firm[i].profit = (1/firm[i].x.cheap)*firm[i].mup*firm[i].u*firm[i].numberOfNewSubMarkets;
			firm[i].bud += firm[i].profit;
			indiv.techlevelCheap +=firm[i].share*firm[i].x.cheap;
			indiv.techlevelPerf +=firm[i].share*firm[i].x.perf;
			showFirmMarket(i);
		}
		if (standard) {
			findStandardShares();
			print("STANDARD SHARES: Apple "+Apple.share+" IBM "+IBM.share+"\n");
		}
		print("HERFINDAL "+indiv.herfindal+ " COMPUTERS SOLD "+indiv.quantityPurchased);

		for(i=1;i<=NN;i++){
		    if((firm[i].alive)&&(firm[i].servedMkt==2)&&(firm[i].generation==2)) indiv.MPshare += firm[i].share;
		}
		for(i=1;i<=NN;i++){
		    if((firm[i].alive)&&(firm[i].servedMkt==2)&&(firm[i].diversified)) indiv.DIVshare += firm[i].share;
		}

	}



	public static void failFirm(int i) {

		firm[i].debt -= firm[i].bud;
		firm[i].bud = 0;
		firm[i].alive = false;
	}




	public static void bookKeeping() {

		int i, numA = 0, numB = 0;
		bfirm.profitRate = 0;
		indiv.profitRate = 0;
		bfirm.maxProfitRate = 0;
		indiv.maxProfitRate = 0;

		print("\nBOOK KEEPING");
		for(i=1;i<=NN;i++) {	  // bookkeeping is calculated for failed firm also
			if (firm[i].debt>0) {
				if ((firm[i].profit>0)&&(t-firm[i].birth>projectDuration)) {
					firm[i].debt -= firm[i].profit * fractionOfProfitForDebtPayBack;
					firm[i].bud -= firm[i].profit * fractionOfProfitForDebtPayBack;
					if (firm[i].debt<0) {  // if paybak is more than debt then the rest returns to budget
						firm[i].bud += -firm[i].debt;
						firm[i].debt = 0;
					}
				}
				firm[i].debt *= (1+r);
			}
			firm[i].bud*=(1+r);
			firm[i].oldProfitRate = firm[i].profitRate;
			firm[i].profitRate = (firm[i].bud-firm[i].debt)/(firm[i].initBud*Math.pow(1+r,t-firm[i].birth));
			firm[i].slopeOfProfitRate = firm[i].slopeOfProfitRate*((wheightOfPast-1)/(wheightOfPast)) + (firm[i].profitRate - firm[i].oldProfitRate) /wheightOfPast;
			showFirmBookeeping(i);
			if (firm[i].servedUserClass == bfirm) {
				numA++;
				bfirm.profitRate += firm[i].profitRate;
				if (numA==1) bfirm.maxProfitRate=firm[i].profitRate; // first value to compare
				else if (firm[i].profitRate>bfirm.maxProfitRate) bfirm.maxProfitRate=firm[i].profitRate;
			}
			if (firm[i].servedUserClass == indiv) {
				numB++;
				indiv.profitRate += firm[i].profitRate;
				if (numB==1) indiv.maxProfitRate=firm[i].profitRate; // first value to compare
				else if (firm[i].profitRate>indiv.maxProfitRate) indiv.maxProfitRate=firm[i].profitRate;
			}
			if ((firm[i].alive)&&(firm[i].entered)&&(firm[i].profitRate<0)&&(firm[i].slopeOfProfitRate<slopeOfFailure)) {
				failFirm(i);
				print(" failed now\n");
			}
		}
		if (numA>0) bfirm.profitRate /= numA; // calculate average profitRate of the market
		if (numB>0) indiv.profitRate /= numB;
		print("AVERAGE PROFITRATES OF THE MARKETS: bfirm "+bfirm.profitRate+" indiv "+indiv.profitRate+"\n");

	}

	public static void secondGenerationCreation() {
		int i;
		double a;
		Mix m = new Mix();
		for (i=firstGeneration+1;i<=secondGeneration+firstGeneration;i++) {
			firm[i] = new Firm(i, false);
	  		firm[i].id = i;
			firm[i].birth = t;
			firm[i].generation = 2;
			firm[i].tec = MP;
			m.randomMix();
			firm[i].mix.cheapMix = m.cheapMix;
			firm[i].mix.perfMix = m.perfMix;
			firm[i].alive = true;
			firm[i].entered = false;
			firm[i].diversified = false;
			firm[i].adopted = false;
			firm[i].daughter = false;
			firm[i].mother = false;
			a = minInitBud + Math.random()*(maxInitBud-minInitBud); // random initial budget
			firm[i].initBud = a * initBudReductionFactor;
			firm[i].debt = a * initBudReductionFactor;
			firm[i].bud = a * initBudReductionFactor;
			firm[i].mup = markUp;
			firm[i].RDExpenditure = RDexpendituresOnProfits;

		}
	}



	public static void main(String arg[]) {


		if (!multi) {
			int i;
			double a;

			try {
				fp = new DataOutputStream(
					new BufferedOutputStream(
						new FileOutputStream("simout.txt")));
			} catch (IOException e) {System.out.println(e.getMessage());}

			paramDefaultValues();

			NN = firstGeneration;
			for(i=1;i<=NN;i++) { // prepares memory for first generation firms
				a = minInitBud + Math.random()*(maxInitBud-minInitBud); // random initial budget
				Mix m = new Mix();
				m.randomMix();
				firm[i] = new Firm(i, 1, 1, TR, m, true, false, false, false, false, false, a, a, a, markUp, RDexpendituresOnProfits);
			}
			System.out.println("\nSINGLE SIMULATION START TIME "+timer());
			for (t=1;t<=end;t++) {
				if (t==secondGenerationTime) {
					NN += secondGeneration;
					secondGenerationCreation();
				}
				print("\nPERIOD "+t+" ------------------------------------------------------");
				if (indiv.quantityPurchased>0)
					if (bfirm.quantityPurchased/indiv.quantityPurchased > goDiversification)
						diversification();
				RDInvest();
				ebwInvest();
				if (t>secondGenerationTime) {
					adoption();
				}
				innovation();
				checkEntry();
				marketBfirm();
				marketIndiv();
				bookKeeping();
				//str = kbdReadString(); if (str.equals("x")) break;
				System.out.print(t+" ");
			}
			System.out.println("\nEND TIME "+timer());

			try {fp.flush(); fp.close();} catch (IOException e) {System.out.println(e.getMessage());}
		}

/* if multi = true the main procedure jump here.

Multi option changes the behavior of the program:
1)	the file output.txt is repleced by file multiout.txt
2)	all print() calls in the NW.class methods produce no effect in this file
3)	disk output is managed by printMulti() method
4)	average history output is written in multiout.txt only at the end of all simulations
5)	multiout.txt file has the following structure:
		a)	a line label for data identification
		b)	the separators are the semicolon and the end of lines
		c)	two lines for every statistic: average values and standard deviations.
		d)	every line has end columns: one for period
		eg.
			Fraction of TRMA leaders in mainf
			1;0.9;0.9;0.7;..;..;..;..;..
			0;0.03;0.02;..;..;..;..;..;..;..
			1;1;0.9;0.8;..;..;..;..;..;..;..;..
			0;0;0.7;..;..;..;..;..;..;..

6)	for every statistic there are two arrays. The first is for M' and the second for M''.
	The second array will contain the stand deviation: dev = sqrt(M'' - M'*M')

*/



		if (multi) {

			double[] avgHERFbfirm = new double[end+1];
			double[] devHERFbfirm = new double[end+1];

			double[] avgHERFindiv = new double[end+1];
			double[] devHERFindiv = new double[end+1];

			double[] avgNUMOFFIRSTGENERATIONbfirm = new double[end+1];
			double[] devNUMOFFIRSTGENERATIONbfirm = new double[end+1];

			double[] avgNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm = new double[end+1];
			double[] devNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm = new double[end+1];

			double[] avgNUMOFSECONDGENERATIONbfirm = new double[end+1];
			double[] devNUMOFSECONDGENERATIONbfirm = new double[end+1];

			double[] avgNUMOFSECONDGENERATIONindiv = new double[end+1];
			double[] devNUMOFSECONDGENERATIONindiv = new double[end+1];

			double[] avgNUMOFDAUGHTERGENERATIONindiv = new double[end+1];
			double[] devNUMOFDAUGHTERGENERATIONindiv = new double[end+1];

			double[] avgLEADERISFIRSTGENERATIONbfirm = new double[end+1];
			// No standard deviation for frequencies
			double[] avgLEADERISSECONDGENERATIONbfirm = new double[end+1];
			// No standard deviation for frequencies

			double[] avgLEADERISSECONDGENERATIONindiv = new double[end+1];
			// No standard deviation for frequencies
			double[] avgLEADERISDAUGHTERGENERATIONindiv = new double[end+1];
			// No standard deviation for frequencies

			double[] avgPROFITRATEbfirm = new double[end+1]; // bfirm market profit rates
			double[] devPROFITRATEbfirm = new double[end+1];


			double[] avgPROFITRATEindiv = new double[end+1]; // indiv market profit rates
			double[] devPROFITRATEindiv = new double[end+1];

			double[] avgMAXPROFITRATEbfirm = new double[end+1]; // bfirm market profit rates
			double[] devMAXPROFITRATEbfirm = new double[end+1];

			double[] avgMAXPROFITRATEindiv = new double[end+1]; // indiv market profit rates
			double[] devMAXPROFITRATEindiv = new double[end+1];

			double[] avgSHAREOFFIRSTGENERATIONLEADERbfirm = new double[end+1];
			double[] devSHAREOFFIRSTGENERATIONLEADERbfirm = new double[end+1];

			double[] avgSHAREOFSECONDGENERATIONLEADERindiv = new double[end+1];
			double[] devSHAREOFSECONDGENERATIONLEADERindiv = new double[end+1];

			double[] avgSHAREOFDAUGHTERLEADERindiv = new double[end+1];
			double[] devSHAREOFDAUGHTERLEADERindiv = new double[end+1];

			double[] avgPROFITRATEOFFIRSTGENERATIONLEADERbfirm = new double[end+1];
			double[] devPROFITRATEOFFIRSTGENERATIONLEADERbfirm = new double[end+1];

			double[] avgPROFITRATEOFSECONDGENERATIONLEADERindiv = new double[end+1];
			double[] devPROFITRATEOFSECONDGENERATIONLEADERindiv = new double[end+1];

			double[] avgPROFITRATEOFDAUGHTERLEADERindiv = new double[end+1];
			double[] devPROFITRATEOFDAUGHTERLEADERindiv = new double[end+1];

			double[] avgDISTANCEOFFIRSTGENERATIONLEADERbfirm = new double[end+1];
			double[] devDISTANCEOFFIRSTGENERATIONLEADERbfirm = new double[end+1];

			double[] avgDISTANCEOFSECONDGENERATIONLEADERindiv = new double[end+1];
			double[] devDISTANCEOFSECONDGENERATIONLEADERindiv = new double[end+1];

			double[] avgDISTANCEOFDAUGHTERLEADERindiv = new double[end+1];
			double[] devDISTANCEOFDAUGHTERLEADERindiv = new double[end+1];

            double[] avgTECHENVIROMENTBfirmPerf = new double[end+1];
            double[] avgTECHENVIROMENTBfirmCheap = new double[end+1];

            double[] avgTECHENVIROMENTIndivPerf = new double[end+1];
            double[] avgTECHENVIROMENTIndivCheap = new double[end+1];

			double[] avgTRfirmMKT1 = new double[end+1];
			double[] avgMPfirmMKT1 = new double[end+1];

			double[] avgMPfirmMKT2 = new double[end+1];
			double[] avgDIVfirmMKT2 = new double[end+1];

			int i, k, w;
			double a;

			try {
				fp = new DataOutputStream(
				new BufferedOutputStream(
						new FileOutputStream("multiout.txt")));
			} catch (IOException e) {System.out.println(e.getMessage());}

			paramDefaultValues(); // here the environmental values are definited
			printParamsOnDisk();

			// k is the counter of repeated simulation in average history
			System.out.println("\nAVERAGE HISTORY START TIME "+timer());
			for (k=1;k<=endOfMulti;k++) {
				paramDefaultValues();
				NN = firstGeneration;
				for(i=1;i<=NN;i++) { // prepares memory for first generation firms
					a = minInitBud + Math.random()*(maxInitBud-minInitBud); // random initial budget
					Mix m = new Mix();
					m.randomMix();
					firm[i] = new Firm(i, 1, 1, TR, m, true, false, false, false, false, false, a, a, a, markUp, RDexpendituresOnProfits);
				}
				for (t=1;t<=end;t++) {
					if (t==secondGenerationTime) {
						NN += secondGeneration;
						secondGenerationCreation();
					}
					print("\nPERIOD "+t+" ------------------------------------------------------");
					if (indiv.quantityPurchased>0)
						if (indiv.quantityPurchased/bfirm.quantityPurchased > goDiversification)
							diversification();
					RDInvest();
					ebwInvest();
					if (t>secondGenerationTime) {
						adoption();
					}
					innovation();
					checkEntry();
					marketBfirm();
					marketIndiv();
					bookKeeping();

					// statistics area: first level
					double xxx;

					xxx = bfirm.herfindal;
					avgHERFbfirm[t] += xxx;
					devHERFbfirm[t] += xxx*xxx;

					xxx = indiv.herfindal;
					avgHERFindiv[t] += xxx;
					devHERFindiv[t] += xxx*xxx;

					xxx = bfirm.numOfFirstGenerationSellingFirms;
					avgNUMOFFIRSTGENERATIONbfirm[t] += xxx;
					devNUMOFFIRSTGENERATIONbfirm[t] += xxx*xxx;

					xxx = bfirm.numOfFirstGenerationDiversifiedSellingFirms;
					avgNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[t] += xxx;
					devNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[t] += xxx*xxx;

					xxx = bfirm.numOfSecondGenerationSellingFirms;
					avgNUMOFSECONDGENERATIONbfirm[t] += xxx;
					devNUMOFSECONDGENERATIONbfirm[t] += xxx*xxx;

					xxx = indiv.numOfSecondGenerationSellingFirms;
					avgNUMOFSECONDGENERATIONindiv[t] += xxx;
					devNUMOFSECONDGENERATIONindiv[t] += xxx*xxx;

					xxx = indiv.numOfDaughterSellingFirms;
					avgNUMOFDAUGHTERGENERATIONindiv[t] += xxx;
					devNUMOFDAUGHTERGENERATIONindiv[t] += xxx*xxx;

					xxx = bfirm.generationOfLeader;
					if (xxx == 1) avgLEADERISFIRSTGENERATIONbfirm[t] += 1;
					if (xxx == 2) avgLEADERISSECONDGENERATIONbfirm[t] += 1;

					xxx = indiv.generationOfLeader;
					if (xxx == 2) avgLEADERISSECONDGENERATIONindiv[t] += 1;
					if (xxx == 3) avgLEADERISDAUGHTERGENERATIONindiv[t] += 1;

					xxx = bfirm.profitRate;
					avgPROFITRATEbfirm[t] += xxx;
					devPROFITRATEbfirm[t] += xxx*xxx;

					xxx = indiv.profitRate;
					avgPROFITRATEindiv[t] += xxx;
					devPROFITRATEindiv[t] += xxx*xxx;

					xxx = bfirm.maxProfitRate;
					avgMAXPROFITRATEbfirm[t] += xxx;
					devMAXPROFITRATEbfirm[t] += xxx*xxx;

					xxx = indiv.maxProfitRate;
					avgMAXPROFITRATEindiv[t] += xxx;
					devMAXPROFITRATEindiv[t] += xxx*xxx;

					xxx = bfirm.shareOfFirstGenerationLeader;
					avgSHAREOFFIRSTGENERATIONLEADERbfirm[t] += xxx;
					devSHAREOFFIRSTGENERATIONLEADERbfirm[t] += xxx*xxx;

					xxx = indiv.shareOfSecondGenerationLeader;
					avgSHAREOFSECONDGENERATIONLEADERindiv[t] += xxx;
					devSHAREOFSECONDGENERATIONLEADERindiv[t] += xxx*xxx;

					xxx = indiv.shareOfDaughterLeader;
					avgSHAREOFDAUGHTERLEADERindiv[t] += xxx;
					devSHAREOFDAUGHTERLEADERindiv[t] += xxx*xxx;

					xxx = bfirm.profitRateOfFirstGenerationLeader;
					avgPROFITRATEOFFIRSTGENERATIONLEADERbfirm[t] += xxx;
					devPROFITRATEOFFIRSTGENERATIONLEADERbfirm[t] += xxx*xxx;

					xxx = indiv.profitRateOfSecondGenerationLeader;
					avgPROFITRATEOFSECONDGENERATIONLEADERindiv[t] += xxx;
					devPROFITRATEOFSECONDGENERATIONLEADERindiv[t] += xxx*xxx;

					xxx = indiv.profitRateOfDaughterLeader;
					avgPROFITRATEOFDAUGHTERLEADERindiv[t] += xxx;
					devPROFITRATEOFDAUGHTERLEADERindiv[t] += xxx*xxx;

					xxx = bfirm.distanceOfFirstGenerationLeader;
					avgDISTANCEOFFIRSTGENERATIONLEADERbfirm[t] += xxx;
					devDISTANCEOFFIRSTGENERATIONLEADERbfirm[t] += xxx*xxx;

					xxx = indiv.distanceOfSecondGenerationLeader;
					avgDISTANCEOFSECONDGENERATIONLEADERindiv[t] += xxx;
					devDISTANCEOFSECONDGENERATIONLEADERindiv[t] += xxx*xxx;

					xxx = indiv.distanceOfDaughterLeader;
					avgDISTANCEOFDAUGHTERLEADERindiv[t] += xxx;
					devDISTANCEOFDAUGHTERLEADERindiv[t] += xxx*xxx;

                    xxx=indiv.techlevelCheap;
                    avgTECHENVIROMENTIndivCheap[t] +=xxx;

                    xxx=indiv.techlevelPerf;
                    avgTECHENVIROMENTIndivPerf[t] +=xxx;

                    xxx=bfirm.techlevelCheap;
                    avgTECHENVIROMENTBfirmCheap[t] +=xxx;

                    xxx=bfirm.techlevelPerf;
                    avgTECHENVIROMENTBfirmPerf[t] +=xxx;

                    xxx=bfirm.TRshare;
                    avgTRfirmMKT1[t] +=xxx;

                    xxx=bfirm.MPshare;
                    avgMPfirmMKT1[t] +=xxx;

                    xxx=indiv.MPshare;
                    avgMPfirmMKT2[t] +=xxx;

                    xxx=indiv.DIVshare;
                    avgDIVfirmMKT2[t] +=xxx;





				} // end of single simulation (t cycle)
				System.out.print(k+" ");


			} // end of average history (k cycle) in multi option

			//statistics area: second level

//===============================================================================
			printMulti("Herfindal index for bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgHERFbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgHERFbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devHERFbfirm[w] = Math.sqrt(devHERFbfirm[w]/endOfMulti - avgHERFbfirm[w]*avgHERFbfirm[w]);
				printMulti(devHERFbfirm[w]+";");
			}
			printMulti("\n");

//===============================================================================
			printMulti("Herfindal index for indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgHERFindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgHERFindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devHERFindiv[w] = Math.sqrt(devHERFindiv[w]/endOfMulti - avgHERFindiv[w]*avgHERFindiv[w]);
				printMulti(devHERFindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Number of first generation firm for bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgNUMOFFIRSTGENERATIONbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgNUMOFFIRSTGENERATIONbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devNUMOFFIRSTGENERATIONbfirm[w] = Math.sqrt(devNUMOFFIRSTGENERATIONbfirm[w]/endOfMulti - avgNUMOFFIRSTGENERATIONbfirm[w]*avgNUMOFFIRSTGENERATIONbfirm[w]);
				printMulti(devNUMOFFIRSTGENERATIONbfirm[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Number of first generation microprocessor firm for bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w] = Math.sqrt(devNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w]/endOfMulti - avgNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w]*avgNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w]);
				printMulti(devNUMOFFIRSTGENERATIONDIVERSIFIEDbfirm[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Number of second generation firm for bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgNUMOFSECONDGENERATIONbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgNUMOFSECONDGENERATIONbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devNUMOFSECONDGENERATIONbfirm[w] = Math.sqrt(devNUMOFSECONDGENERATIONbfirm[w]/endOfMulti - avgNUMOFSECONDGENERATIONbfirm[w]*avgNUMOFSECONDGENERATIONbfirm[w]);
				printMulti(devNUMOFSECONDGENERATIONbfirm[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Number of second generation firm for indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgNUMOFSECONDGENERATIONindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgNUMOFSECONDGENERATIONindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devNUMOFSECONDGENERATIONindiv[w] = Math.sqrt(devNUMOFSECONDGENERATIONindiv[w]/endOfMulti - avgNUMOFSECONDGENERATIONindiv[w]*avgNUMOFSECONDGENERATIONindiv[w]);
				printMulti(devNUMOFSECONDGENERATIONindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Number of daughter firm for indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgNUMOFDAUGHTERGENERATIONindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgNUMOFDAUGHTERGENERATIONindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devNUMOFDAUGHTERGENERATIONindiv[w] = Math.sqrt(devNUMOFDAUGHTERGENERATIONindiv[w]/endOfMulti - avgNUMOFDAUGHTERGENERATIONindiv[w]*avgNUMOFDAUGHTERGENERATIONindiv[w]);
				printMulti(devNUMOFDAUGHTERGENERATIONindiv[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Frequency of leadership for first generation in bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgLEADERISFIRSTGENERATIONbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgLEADERISFIRSTGENERATIONbfirm[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Frequency of leadership for second generation in bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgLEADERISSECONDGENERATIONbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgLEADERISSECONDGENERATIONbfirm[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Frequency of leadership for second generation in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgLEADERISSECONDGENERATIONindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgLEADERISSECONDGENERATIONindiv[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Frequency of leadership for daughter generation in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgLEADERISDAUGHTERGENERATIONindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgLEADERISDAUGHTERGENERATIONindiv[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Average profit rates for bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgPROFITRATEbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgPROFITRATEbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devPROFITRATEbfirm[w] = Math.sqrt(devPROFITRATEbfirm[w]/endOfMulti - avgPROFITRATEbfirm[w]*avgPROFITRATEbfirm[w]);
				printMulti(devPROFITRATEbfirm[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Average profit rates for indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgPROFITRATEindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgPROFITRATEindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devPROFITRATEindiv[w] = Math.sqrt(devPROFITRATEindiv[w]/endOfMulti - avgPROFITRATEindiv[w]*avgPROFITRATEindiv[w]);
				printMulti(devPROFITRATEindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Max profit rate for bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgMAXPROFITRATEbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgMAXPROFITRATEbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devMAXPROFITRATEbfirm[w] = Math.sqrt(devMAXPROFITRATEbfirm[w]/endOfMulti - avgMAXPROFITRATEbfirm[w]*avgMAXPROFITRATEbfirm[w]);
				printMulti(devMAXPROFITRATEbfirm[w]+";");
			}
			printMulti("\n");

//===============================================================================


			printMulti("Max profit rate for indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgMAXPROFITRATEindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgMAXPROFITRATEindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devMAXPROFITRATEindiv[w] = Math.sqrt(devMAXPROFITRATEindiv[w]/endOfMulti - avgMAXPROFITRATEindiv[w]*avgMAXPROFITRATEindiv[w]);
				printMulti(devMAXPROFITRATEindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Share of first generation leader in bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgSHAREOFFIRSTGENERATIONLEADERbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgSHAREOFFIRSTGENERATIONLEADERbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devSHAREOFFIRSTGENERATIONLEADERbfirm[w] = Math.sqrt(devSHAREOFFIRSTGENERATIONLEADERbfirm[w]/endOfMulti - avgSHAREOFFIRSTGENERATIONLEADERbfirm[w]*avgSHAREOFFIRSTGENERATIONLEADERbfirm[w]);
				printMulti(devSHAREOFFIRSTGENERATIONLEADERbfirm[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Share of second generation leader in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgSHAREOFSECONDGENERATIONLEADERindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgSHAREOFSECONDGENERATIONLEADERindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devSHAREOFSECONDGENERATIONLEADERindiv[w] = Math.sqrt(devSHAREOFSECONDGENERATIONLEADERindiv[w]/endOfMulti - avgSHAREOFSECONDGENERATIONLEADERindiv[w]*avgSHAREOFSECONDGENERATIONLEADERindiv[w]);
				printMulti(devSHAREOFSECONDGENERATIONLEADERindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Share of daughter leader in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgSHAREOFDAUGHTERLEADERindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgSHAREOFDAUGHTERLEADERindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devSHAREOFDAUGHTERLEADERindiv[w] = Math.sqrt(devSHAREOFDAUGHTERLEADERindiv[w]/endOfMulti - avgSHAREOFDAUGHTERLEADERindiv[w]*avgSHAREOFDAUGHTERLEADERindiv[w]);
				printMulti(devSHAREOFDAUGHTERLEADERindiv[w]+";");
			}
			printMulti("\n");



//===============================================================================

			printMulti("Profit rate of first generation leader in bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w] = Math.sqrt(devPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w]/endOfMulti - avgPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w]*avgPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w]);
				printMulti(devPROFITRATEOFFIRSTGENERATIONLEADERbfirm[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Profit rate of second generation leader in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgPROFITRATEOFSECONDGENERATIONLEADERindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgPROFITRATEOFSECONDGENERATIONLEADERindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devPROFITRATEOFSECONDGENERATIONLEADERindiv[w] = Math.sqrt(devPROFITRATEOFSECONDGENERATIONLEADERindiv[w]/endOfMulti - avgPROFITRATEOFSECONDGENERATIONLEADERindiv[w]*avgPROFITRATEOFSECONDGENERATIONLEADERindiv[w]);
				printMulti(devPROFITRATEOFSECONDGENERATIONLEADERindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Profit rate of daughter leader in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgPROFITRATEOFDAUGHTERLEADERindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgPROFITRATEOFDAUGHTERLEADERindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devPROFITRATEOFDAUGHTERLEADERindiv[w] = Math.sqrt(devPROFITRATEOFDAUGHTERLEADERindiv[w]/endOfMulti - avgPROFITRATEOFDAUGHTERLEADERindiv[w]*avgPROFITRATEOFDAUGHTERLEADERindiv[w]);
				printMulti(devPROFITRATEOFDAUGHTERLEADERindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Tech position of first generation leader in bfirm mkt\n");
			for (w=1;w<=end;w++) {
				avgDISTANCEOFFIRSTGENERATIONLEADERbfirm[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgDISTANCEOFFIRSTGENERATIONLEADERbfirm[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devDISTANCEOFFIRSTGENERATIONLEADERbfirm[w] = Math.sqrt(devDISTANCEOFFIRSTGENERATIONLEADERbfirm[w]/endOfMulti - avgDISTANCEOFFIRSTGENERATIONLEADERbfirm[w]*avgDISTANCEOFFIRSTGENERATIONLEADERbfirm[w]);
				printMulti(devDISTANCEOFFIRSTGENERATIONLEADERbfirm[w]+";");
			}
			printMulti("\n");


//===============================================================================

			printMulti("Tech position of second generation leader in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgDISTANCEOFSECONDGENERATIONLEADERindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgDISTANCEOFSECONDGENERATIONLEADERindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devDISTANCEOFSECONDGENERATIONLEADERindiv[w] = Math.sqrt(devDISTANCEOFSECONDGENERATIONLEADERindiv[w]/endOfMulti - avgDISTANCEOFSECONDGENERATIONLEADERindiv[w]*avgDISTANCEOFSECONDGENERATIONLEADERindiv[w]);
				printMulti(devDISTANCEOFSECONDGENERATIONLEADERindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================

			printMulti("Tech position of daughter leader in indiv mkt\n");
			for (w=1;w<=end;w++) {
				avgDISTANCEOFDAUGHTERLEADERindiv[w] /= endOfMulti; // calculates average values and standard deviation
				printMulti(avgDISTANCEOFDAUGHTERLEADERindiv[w]+";");
			}
			printMulti("\n");
			for (w=1;w<=end;w++) {
				devDISTANCEOFDAUGHTERLEADERindiv[w] = Math.sqrt(devDISTANCEOFDAUGHTERLEADERindiv[w]/endOfMulti - avgDISTANCEOFDAUGHTERLEADERindiv[w]*avgDISTANCEOFDAUGHTERLEADERindiv[w]);
				printMulti(devDISTANCEOFDAUGHTERLEADERindiv[w]+";");
			}
			printMulti("\n");

//===============================================================================
            printMulti("Average position of Cheapness and performance for Mainframe \n");
            for(w=1;w<=end;w++){
                avgTECHENVIROMENTBfirmCheap[w] /=endOfMulti;
                printMulti(avgTECHENVIROMENTBfirmCheap[w]+";");
            }
            printMulti("\n");

            for(w=1;w<=end;w++){
                avgTECHENVIROMENTBfirmPerf[w] /=endOfMulti;
                printMulti(avgTECHENVIROMENTBfirmPerf[w]+";");
            }
            printMulti("\n");

           printMulti("Average position of Cheapness and performance for PC \n");
            for(w=1;w<=end;w++){
                avgTECHENVIROMENTIndivCheap[w] /=endOfMulti;
                printMulti(avgTECHENVIROMENTIndivCheap[w]+";");
            }
            printMulti("\n");

            for(w=1;w<=end;w++){
                avgTECHENVIROMENTIndivPerf[w] /=endOfMulti;
                printMulti(avgTECHENVIROMENTIndivPerf[w]+";");
            }
            printMulti("\n");
//=========================================================================
            printMulti("Average share of TRfirms in mainframe mkt \n" );
            for(w=1;w<=end;w++){
                avgTRfirmMKT1[w] /=endOfMulti;
                printMulti(avgTRfirmMKT1[w]+";");
            }
            printMulti("\n");

            printMulti("Average share of MPfirms in mainframe mkt \n" );
            for(w=1;w<=end;w++){
                avgMPfirmMKT1[w] /=endOfMulti;
                printMulti(avgMPfirmMKT1[w]+";");
            }
            printMulti("\n");

            printMulti("Average share of MPfirms in PC mkt \n" );
            for(w=1;w<=end;w++){
                avgMPfirmMKT2[w] /=endOfMulti;
                printMulti(avgMPfirmMKT2[w]+";");
            }
            printMulti("\n");

            printMulti("Average share of DIVfirms in PC mkt \n" );
            for(w=1;w<=end;w++){
                avgDIVfirmMKT2[w] /=endOfMulti;
                printMulti(avgDIVfirmMKT2[w]+";");
            }
            printMulti("\n");

//=========================================================================



			try {fp.flush(); fp.close();} catch (IOException e) {System.out.println(e.getMessage());}
			System.out.println("\nEND TIME "+timer());
			//kbdReadString();
		} // end of multi option in main()
	} // end of main() in NW.class
} // end of NW.class in nw0001.java
