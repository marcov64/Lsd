#ColourGradient

This is a simple set of algorithms that introduces to LSD the option to use colour gradients in the lattice.
For information on how to use the lattice in LSD see section ==6.8.21 Lattices: creation and updating== in the LSD manual.

/*	Most of the code is from: 
== http://www.andrewnoske.com/wiki/Code_-_heatmaps_and_color_gradients == */ 

###Usage: 
1. In the main LSD file (fun_\*\*.cpp) include the file ==ColourGradient.cpp== before *MODELBEGIN*.
2. At the beginning of the simulation, initialise the ColourGradient by calling ==ColorGradient::init_colorgradient()==  
3. Also initialise the lattice as described in the manual, by calling ==init lattice(pixW, pixH, nrow, ncol,"LRow" , "LCol", "LVar", obj, color)==.
4. Except that you call one of the following instead of ==update_lattice()== to update a single cell in the lattice:
	a) ==ColorGradient::LSD_lattice_update(line, col, gradient-value)== /* for a red-green-blue gradient */
    b) ==ColorGradient::LSD_lattice_update_Monochrome(line, col, gradient-value)== /* Monochrome */
    c) ==ColorGradient::LSD_lattice_update_RedWhiteGreen(line, col, gradient-value)== /* Red-white-green */
    d) ==ColorGradient::LSD_lattice_update_RedWhiteRed(line, col, gradient-value)== /* Red-white-red (extremes) */
    
The gradient-value needs to be a double in [0,1]. You can use the function ==ColorGradient::normalise_value(v_min,v_max,v)== to normalise the value v with given bounds v_min and v_max to [0,1].

Furthermore, you may also update a cell in the lattice using ==ColorGradient::update_lattice_RGB(line, col, v_r, v_g, v_b)==, where v_r, v_g and v_b are the respective RGB colour values (each in [0,255]).

#####Disclaimer
You may do with the code as you want. I don't take any responsibility for the code. You may reach out to me via email: [frederik.schaff@fernuni-hagen.de](frederik.schaff@fernuni-hagen.de)
