#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <QObject>

const char dataConst[4] {'d','a','t','a'};
//char riffConst[4] {'R','I','F','F'};
//char waveConst[4] {'W','A','V','E'};
//char fmtConst[4]  {'f','m','t',' '};

struct chunk
{
    char        id[4];
    quint32     size;
};

struct RIFFHeader
{
    chunk       descriptor;     // "RIFF"
    char        type[4];        // "WAVE"
};

struct WAVEHeader
{
    chunk       descriptor;
    quint16     audioFormat;
    quint16     numChannels;
    quint32     sampleRate;
    quint32     byteRate;
    quint16     blockAlign;
    quint16     bitsPerSample;
};

struct DATAHeader
{
    chunk       descriptor;
};

struct CombinedHeader
{
    RIFFHeader  riff;
    WAVEHeader  wave;
};

#endif // WAVEFILE_H
