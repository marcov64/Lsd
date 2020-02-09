/*****INNOVATION/IMITATION*****/


EQUATION("Frontier_Productivity")
/*
The new productivity is the maximum among the previous one and the ones possibly obtained imitation and innovation.  
*/
	v[0]=VL("Frontier_Productivity", 1);       //frontier productivity of the firm in the last period
	v[1]=V("Imitation_Productivity");          //productivity achievied with imitation in that period
	v[2]=V("Innovation_Productivity");         //productivity achievied with innovation in that period
	v[3]=max(v[0],max(v[1],v[2]));             //current frontier productivity is the maximum between the three
RESULT(v[3])


EQUATION("Quality")
/*
The new quality is the maximum among the previous one and the ones possibly obtained imitation and innovation. 
*/
	v[0]=VL("Quality", 1);       				//frontier quality of the firm in the last period
	v[1]=V("imitation_quality");          		//quality achievied with imitation in that period
	v[2]=V("innovation_quality");         		//quality achievied with innovation in that period
	v[3]=max(v[0],max(v[1],v[2]));        		//current frontier quality is the maximum between the three
RESULT(v[3])


EQUATION("Imitation_Productivity")
/*
Imitation process. The sucess depends on the amount of recources alocated to imitation. Firms search for best productivity and best quality of the sector, trying to copy if succeded.
*/
	v[0]=V("Revenue");                      	//firm's revenue 
	v[1]=V("imitation_revenue_proportion");     //firm's share of revenue destinated to imitation
	v[2]=(v[0]*v[1]);                           //amount of recources for imitation
	v[3]=1-exp(-v[2]);                   		//probability of success of the imitation depends on amount of recources available
	
	if(RND<v[3])                              	//draws a random number. if it is lower then imitation probability
		{
     	v[4]=VL("Max_Productivity_Sector", 1);  //imitation has succeded and the firm can copy the maximum probability of the sector in the last period
     	v[5]=VL("Max_Quality_Sector", 1);		//imitation has succeded and the firm can copy the maximum quality of the sector in the last period
		}
  	else                                      	//if the random number is not lower than imitation probability
  		{
     	v[4]=0;                                 //imitation failed and return a productivity zero
     	v[5]=0;									//imitation failed and return a quality zero
     	}
    WRITE("imitation_quality", v[5]);			//write the imitation quality
RESULT(v[4])


EQUATION("Innovation_Productivity")
/*
Innovation process. The sucess depends on the amount ou recources alocated to innovation. Firms search for new productivity and porduct quality and the result depends on a random distribution with exonegous parameters.
*/
	v[0]=V("Revenue");                      	//firm's revenue from the last period                         
	v[1]=V("innovation_revenue_proportion");    //firm's share of revenue destinated to innovation
	v[2]=(v[0]*v[1]);                           //amount of recources for innovation
	v[3]=1-exp(-v[2]);                     		//probability of success of the innovation depends on the parameter and the amount of recources available  
	
	if(RND<v[3])                                //draws a random nuumber. if it is lower then innovation probability 
		{
		v[4]=V("std_dev_innovation");           //innovation standard deviation
		v[5]=V("frontier_productivity_initial");//initial frontier productivity
		v[6]=V("quality_initial");				//initial quality
		v[7]=V("tech_opportunity");             //technological opportunity parameter
		v[8]=log(v[5])+(double)t*(v[7]);        //the average of the innovation distribution will be the initial frontier productivity plus the opportunity parameter times the time period
		v[9]=log(v[6])+(double)t*(v[7]);        //the average of the innovation distribution will be the initial quality plus the opportunity parameter times the time period
		v[10]=exp(norm(v[8],v[4]));             //the innovation productivity will be a draw from a normal distribution with average depending of the tech regime and std. dev fixed
		v[11]=exp(norm(v[9],v[4]));				//the innovation quality will be a draw from a normal distribution with average depending of the tech regime and std. dev fixed
		}
	else                                        //if the random number is not lower then  the innovation probability
		{
		v[10]=0;                                //innovation failed and the productivity is zero
		v[11]=0;								//innovation failed and the quality is zero
		}
	WRITE("innovation_quality", v[11]);			//write the innovation quality
RESULT(v[10])













