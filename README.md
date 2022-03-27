# NeuralNetworkGA1
Neural Network with Genetic Algorithms Experiment (food and water) with online demo!

An experiment using Neural Networks and genetic algorithms
I have always found you understand something better if you build it yourself.

# 2022 Update
Since creating this I've created a new demo with new C++ code to explore some different situations.
I recommend watching the video at  https://youtu.be/bq3FdlUeOTU
I've also updated this demo to use more modern javascript. (ES6)
I also fixed a long standing issue with the way they wrap-around the edges of the screen.

My aim was to make a "lifeform" that learnt basic needs.  The lifeform needs water and food to survive.
Water is always available, food is scattered and once eaten re-spawns elsewhere.
If the "lifeform" runs out of food or water then it is classed as "dead"

See it running at https://youtu.be/yZUhprqGVTE
Live demo at http://experiments.robsmithdev.co.uk/NeuralNetworkGA1/

 
There are 30 "lifeforms" in each generation, and each generation runs for a maximum of 6000 iterations.
The project is setup with a Neural Network with 8 inputs, two hidden layers and 2 outputs.

With the current configuration and topology, by the 950th generation 2/3 of the lifeforms survive.

I have included data for the network after I left it running over night (it reached 17120 generations)

-----------------------------------------------------------------------------
I found the following resources useful and inspirational while creating this:

MIT AI Courseware - Worth a watch: https://www.youtube.com/playlist?list=PLUl4u3cNGP63gFHB6xb-kVBiQHYe_4hSi




Copyright © RobSmithDev 2021-2022
https://robsmithdev.co.uk

