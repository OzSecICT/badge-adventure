#include <Arduino.h>

struct Dialog
{
    int id;
    String text;
    String response1;
    int response1_id;
    String response2;
    int response2_id;
};

struct Npc
{
    int id;
    String name;
    const Dialog *dialog;
};

// Dialog object for Simon
const Dialog dialogSimon[] = {
    {0, "Hello, I'm Simon. Have you played a text based adventure before? Games like MUD's and MUSH's of ye olden times?", "Yes, I am familiar, let's get started!", 1, "No, let's have a quick tutorial!", 2},
    {1, "Perfect! I'm sure you can figure it all out then, just remember to try 'help' to see what commands are available.", "Thanks!", 3, "", -1},
    {2, "Alright! I'm sure you already figured out that you type commands to interact. Definitely check out 'help' to see what commands are available to you.", "Ok.", 4, "", -1},
    {3, "Feel free to explore this small tutorial area, you may find something you're looking for. When you're ready to leave, go to talk to Bob outside and let him know you're ready to enter the full game.", "", -1, "", -1},
    {4, "The goal of the game will be to explore, visit some towns, and light up your badge.", "That sounds fun.", 5, "That seems like a lot of work.", 6},
    {5, "It shouldn't be too bad! You can use your commands available to do things like connect to wifi, change your name, save your game.", "Ok..", 6, "", -1},
    {6, "As you visit towns in the game, they corrospond to the lights on your badge. Complete a quest in each town to light it up. End up in Wichita and you can unlock the ultimate flashy mode.", "What's that?", 7, "", -1},
    {7, "Rainbows, of course! RGB! All the lights!", "Well that sounds fun!", 8, "What if I don't like rainbows?", 9},
    {8, "Yeah! Who doesn't like RGB! Anyways, get started by heading to the west by typing 'w' and explore this small area. When you're ready to leave, talk to Bob.", "Ok!", 10, "", -1},
    {9, "Oh.. well.. maybe once you get to Wichita in the game you can choose what color you want that light to be.", "Ok, that is pretty cool.", 10, "", -1},
    {10, "Good luck with the game! Remember to explore this tutorial area and see what you can find, then talk to Bob outside to enter the full game!", "", -1, "", -1}};

// Dialog object for Bob
const Dialog dialogBob[] = {
    {0, "Hi! I'm Bob.", "Hi, Bob.", 1, "I'm ready to leave, Bob.", 4},
    {1, "Would you like a flag?", "Thanks, Bob, I would love a flag!", 2, "Not really.", 3},
    {2, "Here you go: OzSecCTF{B0B_1s_c00l}", "Thanks, Bob.", 3, "I'm ready to leave the tutorial area, Bob.", 4},
    {3, "Have a great day!", "", -1, "", -1},
    {4, "Great news, I trust you found the flag from this area?", "Flag? No..", 5, "Yes I found it!", -2},
    {5, "Ah, you might want to go explore some then, don't worry, this area isn't very big!", "Will do!", 3, "", -1},
    {6, "Excellent! In that case, you can now press that button and you'll be teleported to the full game.", "Thank you, Bob!", 3, "", -1}}; // Quest completed dialog.

const Dialog dialogChanuteReporter[] = {
    {0, "Hello, I'm a reporter from the Chanute Tribune. I'm here to cover the story of the century!", "What's the story?", 1, "I'm not interested.", 4},
    {1, "I heard rumors that Chanute used to go by another name.. do you know what it is?", "No, but I'll find out!", 5, "I do!", -2}, // -2 triggers quest completion check
    {2, "I don't think you found what I am looking for. Try exploring and see if you can run across any signage or anything that may reveal the old name.", "I'll keep looking.", 6, "", -1},
    {3, "Yes! Chanoogle! That must be it!", "Glad to help!", 6, "", -1},
    {4, "Oh, well, I'll be here if you change your mind!", "", -1, "", -1},
    {5, "Come back and let me know what you find!", "", -1, "", -1},
    {6, "Keep me informed.", "", -1, "", -1}};

const Dialog dialogWaterManager[] = { // @todo Check this quest dialog, in particular confirm you can return after loading tapes. I might need an additional -2 quest check.
    {0, "Hello, how can I help you?", "Why are you so worried?", 1, "I am here to help.", 2},
    {1, "We're currently facing a critical situation. Our systems have been compromised by ransomware and everything is offline.", "That's terrible, is there anything I can do to help?", 9, "I actually need to pay my water bill...", 5},
    {2, "Let's see...", "Did that work?", -2, "", -1},
    {3, "Sorry, I don't know what that is but it's not our backup tapes.", "Sorry, what tapes did you need?", 6, "", -1},
    {4, "That is exactly what we needed!", "I got the tapes, what now?", 10, "I loaded the tapes, what now?", 7},
    {5, "Oh, that's unfortunate, we can't take anything right now. But rest assured, we will bill you when we're back online.", "", -1, "", -1},
    {6, "The tapes are located in our safe deposit box at the bank on Main Street. Tell the staff that I sent you and they can get you the tapes. Bring them back here as soon as you can!", "I'll get the tapes and return as soon as possible.", 7, "", -1},
    {7, "That's all, thank you for helping!", "", -1, "", -1},
    {8, "Do we still need the tapes, or have you already loaded them up?", "Are these the right tapes?", 2, "I already loaded them.", 2},
    {9, "We have air gapped backup tapes stored in a safe deposit box at the bank. We need someone to retrieve them for us. Can you do that?", "I can do that.", 6, "", -1},
    {10, "Here's a keycard to get you into the data center across the hall. Can you go load them up into the backup drive for me?", "I can do that.", 7, "", -1}};

const Dialog dialogWaterReceptionist[] = {
    {0, "Hello, how can I help you?", "You look pre-occupied.", 1, "I need to pay my water bill, but I can't login to your website.", 2},
    {1, "We're in a bit of a pickle at the moment.", "I am sure I can help.", 3, "What's going on?", 3},
    {2, "Oh yes, our systems are all down at the moment, and we can't take any payments. Say, you seem smart, do you think you could help us with something?", "Yes, I can help.", 1, "No, I have some other things to do first.", 4},
    {3, "Go check with our IT Manager, he's in the office to the right straight ahead. Thank you!", "I'll go see him, thank you.", 5, "", -1},
    {4, "Oh, well, if you change your mind, I'll be here.", "", -1, "", -1},
    {5, "No, thank you!", "", -1, "", -1}};

const Dialog dialogBankTeller[] = {
    {0, "Hello, how can I help you?", "I need to access a safe deposit box.", 1, "This is an emergency. I need to get into the Water Co's deposit box!", 1},
    {1, "Are you authorized by the Water Company to access their box?", "Yes, I am.", -2, "The IT Manager sent me here.", -2},
    {2, "", "", -1, "", -1},
    {3, "That works, here is the key. The boxes are in the vault.", "", -1, "", -1}};

const Dialog dialogPittsburgTownHall[] = {
    {0, "We're in a bit of a pickle here, dear. Everything's down, phones, email, internet, everything!", "I think I can help.", 3, "Can you check again?", -2},
    {1, "I just did, nothing is working. Think you can do something about this? We have all these experts around and all they care about is scope and insurance!", "I absolutely can.", 3, "", -1},
    {2, "I can't beleive it! Everything is coming back up! You did what consultants and experts couldn't do! Thank you so much.", "I'm glad I could help.", 5, "", -1},
    {3, "I don't know anything about technology, but this letter here desperately needs to get to the courthouse if you could deliver that.", "I'll go check it out.", 4, "", -1},
    {4, "Thank you for your help! If you happen to fix our computers, be sure you come let me know!", "", -1, "", -1},
    {5, "Thank you for your help! I'm going to take a break now.", "", -1, "", -1}};

const Dialog dialogPittsburgCourthouse[] = {
    {0, "Hello, how can I help you?", "I have something for you.", -2, "Do you know what's going on with the city?", 3},
    {1, "Hm, nope, not for me. Thank you though.", "Sorry about that. Do you know what's going on?", 3, "", -1},
    {2, "Excellent! Another letter for the courts. Thank you so much for delivering this!", "", -1, "", -1},
    {3, "Someone is hacking the whole city. Our network gear has been OK for now, but we did see someone in a hoodie running to the park over to the east.", "I'll go check it out.", 4, "", -1},
    {4, "Thank you for your help! If you happen to fix the city computers, go let town hall know. I am expecting an email from them.", "", -1, "", -1}};

const Dialog dialogPittsburgStacey[] = {
    {0, "Hey there. This whole town is falling apart ever since that cyber attack took down the entire city network.", "What's going on?", 1, "Did you order a coffee?", 3},
    {1, "It's been a nightmare. I've been trying to get the city network back online, but the attackers have complete network access from somewhere. It's like they've jacked in from somewhere local.", "Who has that kind of network connectivity?", 2, "", -1},
    {2, "Courts? No, post office? I don't think so. Maybe the university? They have a real nice computer lab..", "I'll go check out the university, see what I can find.", 6, "", -1},
    {3, "You know what, I did, but I have been so busy I forgot to go pick it up.", "I have it right here!", -2, "I'll see if I can go get it for you.", 6},
    {4, "No, that's not my coffee, but thank you for trying. I ordered it from Root Coffeehouse up on Broadway.", "I'll go see if I can find it.", 6, "", -1},
    {5, "Thank you for your help! I'm going to take a much needed coffee break now!", "", -1, "", -1},
    {6, "Thank you.", "", -1, "", -1}};

const Dialog dialogPittsburgDean[] = {
    {0, "I'm sorry, I can't talk right now.", "Is everything OK?", 1, "", -1},
    {1, "Not really, we hired this new IT guy and he's been a real pain in the neck. He's been trying to get us to upgrade our network gear and now everything's offline just like the city.", "Who is the IT guy?", 2, "I'll go see if I can help.", 3},
    {2, "They were affordable, that's who they were. However it's all pointless if everything is down. Plus he's now locked us out of the computer lab.", "I'll go see if I can help.", 3, "", -1},
    {3, "Thank you for your anything you can do!", "", -1, "", -1}};

const Dialog dialogWichitaAirport[] = {
    {0, "Hey, I'm just trying to take a break here.", "You look tired, can I help?", 3, "Is everything OK now?", -2},
    {1, "Not yet, there's still some PC's that are stuck in reboot loops around the airport. Can you go reboot them with the recovery drive?", "Sure.", 5, "", -1},
    {2, "Yes! My monitoring is showing everything back online! Thank you so much for helping run those PC's down!", "Any time.", 6, "", -1},
    {3, "That would be wonderful, all of our signage and PC's have been offline all day. It's pretty bad here. I have to physically touch every single PC that is currently stuck in a reboot loop.", "That's terrible, how can I help?", 4, "That sounds familiar, I think I know what to do.", 4},
    {4, "There's a recovery drive on this table, grab that, stick it in any PC's you find that are stuck, and reboot them. It will take care of the rest.", "I'll get started right away.", 5, "", -1},
    {5, "Thank you for your help! Come let me know when you're done.", "", -1, "", -1},
    {6, "Thank you for your help! I'm going to take a break now.", "", -1, "", -1}};

const Dialog dialogGoodlandRepair[] = {
    {0, "Hey, we're a bit busy right now, can you help?", "Sure, what's wrong?", 1, "", -1},
    {1, "We are tasked with repairing all these robot taxi's that took over the town. I heard that some of them are stuck in the streets. Can you take this key and get them going again?", "I can do that.", 2, "", -1},
    {2, "Thanks! You will probably need to update their firmware, check with the Telecom service on 13th street, they can get you a drive to update the taxi.", "I will.", -2, "", -1},
    {3, "Eh, no one should see this message.", "", -1, "", -1},
    {4, "Thanks, it would really help us out. Once you've repaired them all, go check with Telecom to see if they need anything else.", "", -1, "", -1}};

const Dialog dialogGoodlandTelecom[] = {
    {0, "Hey you! You look techy, can you go flash some taxi's? We have a crisis here. They are going haywire.", "Yes", 3, "I already did!", -2},
    {1, "Looks like we are still showing some taxi's are stuck around town. Can you go make sure you fix all of them?", "", -1, "", -1},
    {2, "That's great! Thank you so much for helping us get back online! Looks like we are fully functional now.", "", -1, "", -1},
    {3, "Great! Here's a drive to update the firmware on the taxi's. Go plug it in and get them back on the road.", "I'll get started right away.", -2, "", -1},
    {4, "Thank you for your help! I'm going to take a break now.", "", -1, "", -1},
    {5, "Thank you for your help! I'm going to take a break now.", "", -1, "", -1}};

const Dialog dialogBusManager[] = {
    {0, "Oh, my. What are you doing here?", "I need to get to Wichita, there's a security conference I want to attend.", 1, "I fixed some buses in the garage.", 4},
    {1, "I see, well right now we are dealing with a situation that has taken down all our buses. All our maintenance techs are busy, can you help make a few buses road worthy?", "Absolutely!", 3, "Maybe, what's it entail?", 3},
    {2, "Come back to me when you're done.", "", -1, "", -1},
    {3, "We have three buses down in the garage, all needing some work to get them back up. Simple things, like a new air filter. Can you go take care of those?", "", -1, "", -1},
    {4, "Oh perfect, you got them all functional?", "Yes.", -2, "", -1},
    {5, "Looks like a few of them didn't get fixed, can you go check on them all again?.", "Ok.", 2, "", -1},
    {6, "You got them, thank you so much! Here's a ticket to get you on your way to Topeka.", "", -1, "", -1}};

const Dialog dialogPrincipal[] = {
    {0, "Ah, glad you could make it! I wanted to talk to you about something important.",
     "Sure, what's up?", 1, "", -1},
    {1, "We've been experiencing some issues with our network lately, and I think it's best if you meet with our IT Specialist.",
     "Okay, where can I find them?", 2, "", -1},
    {2, "They're in the IT Office northwest of the Commons area. You can't miss it; it's right before the intersection of \"A\" and \"B\" Hall.",
     "Got it. What kind of issues are we dealing with?", 3, "", -1},
    {3, "Just some technical glitches affecting a few classrooms. I'm sure the IT Specialist can give you a detailed rundown.",
     "Thanks for the heads-up! I'll head there now.", 4, "", -1},
    {4, "No problem! Good luck!",
     "", -1, "", -1}};

const Dialog dialogitSpecialist[] = {
    {0, "Hey there! Thanks for coming by. We've got a situation on our hands.",
     "What's going on?", 1, "Where you needing this?", 7},
    {1, "After the cyber attack, someone sent me a message saying they ripped my recovery notebook into pieces and spread them around the school.",
     "That's not good. How can I help?", 2, "", -1},
    {2, "I have part 5 of the recovery information, but we're missing the first four parts. I need you to find them if we're going to get our systems back up.",
     "Got it. I'll start looking for them.", 3, "Is there anything else you need?", 4},
    {3, "Thank you. I appreciate the help. Just bring the pieces back here as you find them.",
     "", -1, "", -1},
    {4, "Actually, if you could bring me my lunch from the staff lounge while you're at it, that would be great. It's been a long day.",
     "Sure thing, I'll grab your lunch too.", 5, "Finding the notebook pieces is my priority, but I'll see if I can grab it.", 6},
    {5, "Perfect. I owe you one! I'll be here working on what I can in the meantime.",
     "", -1, "", -1},
    {6, "No problem. The notebook pieces are the priority. Thanks for helping out.",
     "", -1, "", -1},
    // Player returns with fewer than 4 pieces
    {7, "Did you manage to find any of the missing pages?",
     "Are these what you needed?", -2, "", -1},
    {8, "You're missing a few pages, but it's a great start! Keep searching for the rest, and let me know as soon as you have them all.",
     "", -1, "", -1},
    {9, "Fantastic! This is exactly what we needed. With these, we can start the recovery process. Thanks for your help.",
     "Is there anything else?", 12, "", -1},
    // Player returns with all 4 pieces
    {10, "Have a great day!",
     "", -1, "", -1},
    {11, "Fantastic! This is exactly what we needed. With these, we can start the recovery process. Thanks for your help.",
     "Glad I could help. Anything else?", 12, "No problem, happy to assist.", 12},
    {12, "Actually, if you haven't already, I could still use that lunch from the staff lounge. But no rush—getting the system back up is the priority.",
     "I'll grab it for you now.", 10, "I'll get to it when I can.", 10}};

// Quest completion check -2, dialogIndex +1 if quest failed, +2 if quest completed.
// Array of all NPC objects, their name, and their dialog variable
const Npc npc[] = {{0, "Simon", dialogSimon},
                   {1, "Bob", dialogBob},
                   {2, "Chanute Reporter", dialogChanuteReporter},
                   {3, "IT Manager", dialogWaterManager},
                   {4, "Receptionist", dialogWaterReceptionist},
                   {5, "Bank Teller", dialogBankTeller},
                   {6, "Receptionist", dialogPittsburgTownHall},
                   {7, "Clerk", dialogPittsburgCourthouse},
                   {8, "Stacey", dialogPittsburgStacey},
                   {9, "Dean", dialogPittsburgDean},
                   {10, "Airport IT Guy", dialogWichitaAirport},
                   {11, "Repair Tech", dialogGoodlandRepair},
                   {12, "Telecom Tech", dialogGoodlandTelecom},
                   {13, "Bus Manager", dialogBusManager},
                   {14, "Principal", dialogPrincipal},
                   {15, "IT Specialist", dialogitSpecialist}};
