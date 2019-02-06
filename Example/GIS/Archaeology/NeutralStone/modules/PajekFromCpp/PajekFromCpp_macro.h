/* Some macros for simple usage in LSD and elsewhere
   Obviously, Pajek can also be used directly. In this case, the macros
   highlight the relvant APIs and how to use the Pajek library.
*/

/*
  || Dynamic Network Mode - .paj format  ||
     PAJ_MAKE_AVAILABLE creates at global (to the following commands) space a new, clean Pajek object.
     PAJ_INIT / PAJ_INIT_ANIM initialise the file.
     PAJ_ADD_V* Add vertices
     PAJ_ADD_E* Add Edges
     PAJ_ADD_A* Add Arcs
     PAJ_SAVE Save all information to file.

     The Class will be deconstructed automatically when it goes out of scope if
     you use the macros. Otherwise, you need to take care (e.g. if new is used)

     Notes: Time must always be increased. Vertices need to exist prior to
      linking them. All Information needs to be provided for each time again.

     For debugging use the switch:
    #define PAJEK_CONSISTENCY_CHECK_ON
     before including this file.

  || Static Network Mode ||
     use PAJ_STATIC to create the Pajek object and initialise it at the
      current (local) scope instead of the PAJ_MAKE_AVAILABLE and PAJ_INIT

     Static version of the other commands comannds:
     PAJ_S_ADD_V* Add vertices
     PAJ_S_ADD_E* Add Edges
     PAJ_S_ADD_A* Add Arcs
     PAJ_S_SAVE   Save to file

     The file that is created also holds information of the time (*_t123.net)
     When the current (local) scope is left, the Pajek object is destroyed.

  || Additionl stuff ||

    The SVG coordinate system starts top left. Instead, standard coordinates
    start bottom left. This is accomplished by reflecting the coordinates and
    then shifting them by one. If your coordinates follow the original SVG
    specificaton
  #define Y0isLow false
    before loading this library.

*/

//dynamic macros
#define PAJ_MAKE_AVAILABLE Pajek pajek_core_object;

#define PAJ_INIT(ParentFolderName,SetName,SetID,NetName) 	      pajek_core_object.init(ParentFolderName,SetName,SetID,NetName);
#define PAJ_INIT_ANIM(ParentFolderName,SetName,SetID,NetName) 	pajek_core_object.init(ParentFolderName,SetName,SetID,NetName,true);

#define PAJ_SAVE \
    pajek_core_object.save_to_file();

#define PAJ_ADD_V_CL(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR,LABEL)   pajek_core_object.add_vertice(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR,LABEL);

#define PAJ_ADD_V_C(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR)  PAJ_ADD_V_CL(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR,"")
#define PAJ_ADD_E_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,COLOR)	pajek_core_object.add_relation(TIME,sID,sKIND,tID,tKIND,true ,RELnAME,VALUE,WIDTH,COLOR);
#define PAJ_ADD_A_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,COLOR)	pajek_core_object.add_relation(TIME,sID,sKIND,tID,tKIND,false,RELnAME,VALUE,WIDTH,COLOR);

#define PAJ_ADD_V(TIME,ID,KIND,VALUE)         PAJ_ADD_V_C(TIME,ID,KIND,VALUE,0.5,0.5,"ellipse",1.0,1.0,"Black");
#define PAJ_ADD_V_XY(TIME,ID,KIND,VALUE,X,Y)  PAJ_ADD_V_C(TIME,ID,KIND,VALUE,X,Y,"ellipse",1.0,1.0,"Black");

#define PAJ_ADD_E(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME)	        PAJ_ADD_E_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,1.0,"Red");
#define PAJ_ADD_E_W(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH)	PAJ_ADD_E_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,"Red");

#define PAJ_ADD_A(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME)	        PAJ_ADD_A_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,1.0,"Blue");
#define PAJ_ADD_A_W(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH)	PAJ_ADD_A_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,"Blue");

// and the static options

#define PAJ_STATIC(ParentFolderName,SetName,SetID,NetName)  Pajek pajek_core_object_static(ParentFolderName,SetName,SetID,NetName,false,true);

#define PAJ_S_SAVE    pajek_core_object_static.save_to_file();

#define PAJ_S_ADD_V_CL(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR,LABEL)  pajek_core_object_static.add_vertice(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR,LABEL);

#define PAJ_S_ADD_V_C(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR)  PAJ_S_ADD_V_CL(TIME,ID,KIND,VALUE,X,Y,SYMBOL,X_FACT,Y_FACT,COLOR,"")
#define PAJ_S_ADD_E_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,COLOR)	pajek_core_object_static.add_relation(TIME,sID,sKIND,tID,tKIND,true ,RELnAME,VALUE,WIDTH,COLOR);
#define PAJ_S_ADD_A_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,COLOR)	pajek_core_object_static.add_relation(TIME,sID,sKIND,tID,tKIND,false,RELnAME,VALUE,WIDTH,COLOR);

#define PAJ_S_ADD_V(TIME,ID,KIND,VALUE)         PAJ_S_ADD_V_C(TIME,ID,KIND,VALUE,0.5,0.5,"ellipse",1.0,1.0,"Black");
#define PAJ_S_ADD_V_XY(TIME,ID,KIND,VALUE,X,Y)  PAJ_S_ADD_V_C(TIME,ID,KIND,VALUE,X,Y,"ellipse",1.0,1.0,"Black");

#define PAJ_S_ADD_E(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME)	        PAJ_S_ADD_E_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,1.0,"Red");
#define PAJ_S_ADD_E_W(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH)	PAJ_S_ADD_E_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,"Red");

#define PAJ_S_ADD_A(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME)	        PAJ_S_ADD_A_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,1.0,"Blue");
#define PAJ_S_ADD_A_W(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH)	PAJ_S_ADD_A_C(TIME,sID,sKIND,tID,tKIND,VALUE,RELnAME,WIDTH,"Blue");
