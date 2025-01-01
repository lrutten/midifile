#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <set>
#include "Binasc.h"
#include "MidiFile.h"
#include "Options.h"

//using namespace std;
//using namespace smf;

class Event
{
public:
   Event(smf::MidiEvent *evv);
   ~Event();
   const int getTick();
   const int getTickOff();
   bool operator<(const Event *ev2) const
   {
      std::cout << "sort\n";
      return ev->tick < ev2->ev->tick;
   }

   const smf::MidiEvent *getEv()
   {
      return ev;
   }

private:
   smf::MidiEvent *ev;
   smf::MidiEvent *ev_off;
};


template<>
struct std::less<Event *>
{
   bool operator()(Event *e1, Event *e2) const
   {
      return e1->getTick() < e2->getTick();
   }
};



Event::Event(smf::MidiEvent *evv) : ev(evv) 
{
   if (!evv->isLinked())
   {
      std::cerr << "Error: event not linked\n";
   }
   ev_off = evv->getLinkedEvent();
}

Event::~Event()
{
}

const int Event::getTick()
{
   return ev->tick;
}

const int Event::getTickOff()
{
   return ev_off->tick;
}

class Notes
{
public:
   void add_note(smf::MidiEvent *note);
   void show();

private:
   std::map<int, std::set<Event *>> notes;
};

void Notes::add_note(smf::MidiEvent *note)
{
   //std::vector<smf::MidiEvent *> v{};

   //std::cout << "key " << note->getKeyNumber() << "\n";
   int note_nr = note->getKeyNumber();
   //std::vector<smf::MidiEvent *> events = notes.at(note_nr);
   if (!notes.contains(note_nr))
   {
      //std::cout << "contains not " << note_nr << "\n";

      //notes[note_nr] = {};
      notes.insert({note_nr, {}});
   }
   else
   {
      //std::cout << "contains " << note_nr << "\n";
   }
   notes.at(note_nr).insert(new Event(note));
}

void Notes::show()
{
   std::cout << "----- Notes ------\n";
   for (const auto &[note, events]: notes)
   {
      std::string key_s = smf::Binasc::keyToPitchName(note);
      std::cout << "note " << note << " " << key_s <<"\n";
      
      Event *last = nullptr;
      for (int i = 0; Event *ev: events)
      {
         if (i == 0)
         {
            last = ev;
         }
         
         std::cout << "   " << ev->getTick() << " " << ev->getTickOff() << "\n";
         if (i>0 && ev->getTick() < last->getTickOff())
         {
            std::cout << "      " << "overlap\n";
         }
         i++;
      }
   }
}

int main(int argc, char** argv) 
{
   smf::Options options;
   options.process(argc, argv);
   
   smf::MidiFile midifile;
   Notes         notes;

   if (options.getArgCount() == 0)
   {
      midifile.read(std::cin);
   }
   else
   {
      midifile.read(options.getArg(1));
   }
   midifile.doTimeAnalysis();
   midifile.linkNotePairs();

   int tracks = midifile.getTrackCount();
   std::cout << "TPQ: " << midifile.getTicksPerQuarterNote() << std::endl;
   if (tracks > 1)
   {
      //std::cout << "TRACKS: " << tracks << std::endl;
   }
   for (int track=0; track<tracks; track++) 
   {
      if (tracks > 1)
      {
         //std::cout << "\nTrack " << track << std::endl;
      }
      //std::cout << "Tick\tSeconds\tDur\tMessage" << std::endl;
      for (unsigned int event=0; event<midifile[track].size(); event++) 
      {
         //std::cout << std::dec << midifile[track][event].tick;
         //std::cout << '\t' << std::dec << midifile[track][event].seconds;
         //std::cout << '\t';

         if (midifile[track][event].isNoteOn() || midifile[track][event].isNoteOff())
         {
            //std::cout << midifile[track][event].getDurationInSeconds();
            notes.add_note(&midifile[track][event]);
         }
         //std::cout << '\t' << std::hex;
         for (int i=0; i<midifile[track][event].size(); i++)
         {
            //std::cout << (int)midifile[track][event][i] << '-';
         }
         //std::cout << std::endl;
      }
   }

   notes.show();

   return 0;
}
