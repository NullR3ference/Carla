/*
  ZynAddSubFX - a software synthesizer

  FilterParams.h - Parameters for filter
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
*/

#ifndef FILTER_PARAMS_H
#define FILTER_PARAMS_H

#include "../globals.h"
#include "../Misc/XMLwrapper.h"
#include "PresetsArray.h"

class FilterParams:public PresetsArray
{
    public:
        FilterParams(const AbsTime *time_ = nullptr);
        FilterParams(unsigned char Ptype_,
                     unsigned char Pfreq,
                     unsigned char Pq_,
                     const AbsTime *time_ = nullptr);
        ~FilterParams();

        void add2XML(XMLwrapper& xml);
        void add2XMLsection(XMLwrapper& xml, int n);
        void defaults();
        void getfromXML(XMLwrapper& xml);
        void getfromXMLsection(XMLwrapper& xml, int n);
        void paste(FilterParams &);
        void pasteArray(FilterParams &, int section);


        void getfromFilterParams(FilterParams *pars);

        float getfreq() const ;
        float getq() const ;
        float getfreqtracking(float notefreq) const ;
        float getgain() const ;

        unsigned char Pcategory; //Filter category (Analog/Formant/StVar)
        unsigned char Ptype; // Filter type  (for analog lpf,hpf,bpf..)
        unsigned char Pfreq; // Frequency (64-central frequency)
        unsigned char Pq; // Q parameters (resonance or bandwidth)
        unsigned char Pstages; //filter stages+1
        unsigned char Pfreqtrack; //how the filter frequency is changing according the note frequency
        unsigned char Pgain; //filter's output gain

        //Formant filter parameters
        unsigned char Pnumformants; //how many formants are used
        unsigned char Pformantslowness; //how slow varies the formants
        unsigned char Pvowelclearness; //how vowels are kept clean (how much try to avoid "mixed" vowels)
        unsigned char Pcenterfreq, Poctavesfreq; //the center frequency of the res. func., and the number of octaves

        struct Pvowels_t {
            struct formants_t {
                unsigned char freq, amp, q; //frequency,amplitude,Q
            } formants[FF_MAX_FORMANTS];
        } Pvowels[FF_MAX_VOWELS];

        unsigned char Psequencesize; //how many vowels are in the sequence
        unsigned char Psequencestretch; //how the sequence is stretched (how the input from filter envelopes/LFOs/etc. is "stretched")
        unsigned char Psequencereversed; //if the input from filter envelopes/LFOs/etc. is reversed(negated)
        struct {
            unsigned char nvowel; //the vowel from the position
        } Psequence[FF_MAX_SEQUENCE];

        float getcenterfreq() const ;
        float getoctavesfreq() const ;
        float getfreqpos(float freq) const ;
        float getfreqx(float x) const ;

        float getformantfreq(unsigned char freq) const ;
        float getformantamp(unsigned char amp) const ;
        float getformantq(unsigned char q) const ;

        void defaults(int n);


        bool changed;

        const AbsTime *time;
        int64_t last_update_timestamp; // timestamp of last update to this structure,
        // including any change to the vowels/formats

        static const rtosc::Ports ports;
    private:

        //stored default parameters
        unsigned char Dtype;
        unsigned char Dfreq;
        unsigned char Dq;
};

#endif
