#include <vector>
#pragma once
namespace EA {
    namespace Allocator {
        class ICoreAllocator
        {
        public:
            struct DebugParams
            {
                const char* const mName;
                const char* const mFileName;
                const int mLineNum;
            };
            virtual ~ICoreAllocator();
            virtual void* Alloc(unsigned int size, const char* name, unsigned int flags, unsigned int align, unsigned int alignOffset);
            virtual void* Alloc(unsigned int size, const char* name, unsigned int flags);
            virtual void Free(void* block, unsigned int size);
            virtual void* AllocDebug(unsigned int size, DebugParams debugParams, unsigned int flags, unsigned int align, unsigned int alignOffset);
            virtual void* AllocDebug(unsigned int size, DebugParams debugParams, unsigned int flags);
        };
    }
}

namespace rw {
    namespace audio {
        namespace core {
            class DecoderRegistry;
            class EncoderRegistry;
            class StackAllocator;
            struct ListDNode
            {
            private:
                ListDNode* pnext;
                ListDNode* pprev;
            public:
                void SetNext(ListDNode* pnode)
                {
                    pnext = pnode;
                }
                void SetPrev(ListDNode* pnode)
                {
                    pprev = pnode;
                }
                ListDNode* GetNext()
                {
                    return pnext;
                }
                ListDNode* GetPrev()
                {
                    return pprev;
                }
            };
            struct ListDStack
            {
            private:
                ListDNode* phead;
            public:
                ListDStack() : phead(nullptr)
                {
                }
                void Reset()
                {
                    phead = nullptr;
                }
                bool IsEmpty()
                {
                    return phead == nullptr;
                }
                ListDNode* GetHead()
                {
                    return phead;
                }
                void Push(ListDNode* pnode)
                {
                    pnode->SetNext(phead);
                    pnode->SetPrev(nullptr);
                    if (phead)
                    {
                        phead->SetPrev(pnode);
                    }
                    phead = pnode;
                }
                ListDNode* Pop()
                {
                    ListDNode* node = phead;
                    if (node)
                    {
                        phead = node->GetNext();
                        if (node->GetNext() != nullptr)
                        {
                            node->GetNext()->SetPrev(nullptr);
                        }
                    }
                    return node;
                }
                void Remove(ListDNode* pnode)
                {
                    if (pnode == phead)
                    {
                        phead = phead->GetNext();
                    }
                    if (pnode->GetPrev())
                    {
                        pnode->GetPrev()->SetNext(pnode->GetNext());
                    }
                    if (pnode->GetNext())
                    {
                        pnode->GetNext()->SetPrev(pnode->GetPrev());
                    }
                }
            };
            class PlugIn;
            class PlugInRegistry;
            class System {
            private:
                static System* spInstance;
                StackAllocator* mpStackAllocator;
                void(*mpAssertImplementation)(System*, const char*, const char*, const char*, int);
                double mSystemTime;
                ListDStack mExpelledVoiceList;
                EA::Allocator::ICoreAllocator* mpAllocator;
                void* (*mpPhysicalAlloc)(unsigned int, unsigned int, unsigned int, const char*);
                void(*mpPhysicalFree)(void*);
                char* mpCommandBuffer;
                PlugIn* mpMasteringSubMix;
                PlugInRegistry* mpPlugInRegistry;
                DecoderRegistry* mpDecoderRegistry;
                EncoderRegistry* mpEncoderRegistry;
            public:
                System();
                static System* GetInstance() { return spInstance; }
                static void CreateInstance(EA::Allocator::ICoreAllocator* pAllocator);
                static void Assert(System*, const char*, const char*, const char*, int);
                void* Alloc(unsigned int size, char const* name, unsigned int alignment, EA::Allocator::ICoreAllocator* pAllocator);
                void Free(void* pMem, EA::Allocator::ICoreAllocator* pAllocator);
                void Lock();
                void Unlock();
                DecoderRegistry* GetDecoderRegistry();
                EncoderRegistry* GetEncoderRegistry();
                void Release();
            };
            class SampleBuffer
            {
            public:
                static unsigned int GetSize(unsigned int numChannels, unsigned int size, unsigned int* pAlignment, rw::audio::core::System* pSystem)
                {
                    *pAlignment = 16;
                    return 20;
                }
                static SampleBuffer* CreateInstance(unsigned int numChannels, unsigned int maxLocks, unsigned int maxSamples, void* pMem, void* pStorage, rw::audio::core::System* pSystem)
                {
                    SampleBuffer* buf = (SampleBuffer*)pMem;
                    buf->mNumChannels = numChannels;
                    buf->mpSystem = pSystem;
                    buf->mpStorage = (float*)pStorage;
                    buf->mNumSamples = 0;
                    buf->mMaxSamples = maxSamples;
                    return buf;
                }
                static unsigned int CalculateStorageSize(unsigned int numChannels, unsigned int maxSamples)
                {
                    return 4 * numChannels * maxSamples;
                }
                float* LockChannel(unsigned int channel)
                {
                    return &mpStorage[mMaxSamples * channel];
                }
                void UnlockChannel(unsigned int channel)
                {
                }
                void SetNumSamples(unsigned int numSamples)
                {
                    mNumSamples = numSamples;
                }
                unsigned int GetNumSamples()
                {
                    return mNumSamples;
                }
            private:
                System* mpSystem;
                float* mpStorage;
                float* mpTempStore;
                unsigned short mNumSamples;
                unsigned short mMaxSamples;
                char mNumChannels;
            };
            class Decoder
            {
            public:
                struct RequestDesc
                {
                    void* pSrc;
                    void* pSeekData;
                    int decoderSkip;
                    int numSamples;
                    int numBytes;
                    unsigned char feedType;
                    unsigned char seekDataVersion;
                };
                enum FeedType
                {
                    FEEDTYPE_NEW = 0x0,
                    FEEDTYPE_CONTINUE = 0x1,
                };
                virtual void FeedEvent(unsigned char) {}
                virtual ~Decoder() {}
                unsigned char Feed(void*, int, FeedType, int, void*, int, int);
                int Decode(SampleBuffer*, int);
                void Release();
                int GetSamplesRemaining(unsigned char);
                unsigned int GetInstanceSize() { return mInstanceSize; }
                System* GetSystem() { return mpSystemUseGetSystemAccessor; }
                unsigned int GetChannels() { return mNumChannels; }
                Decoder() {}
                void SetSystem(System* system) { mpSystemUseGetSystemAccessor = system; }
                RequestDesc* GetRequestDescArray() { return (RequestDesc*)((char*)this + mRequestDescOffset); }
                RequestDesc* GetRequestDesc(unsigned int slot) { return (RequestDesc*)((char*)this + sizeof(RequestDesc) * slot + mRequestDescOffset); }
                void SetChannels(unsigned int numChannels) { mNumChannels = numChannels; }
                SampleBuffer* GetSampleBuffer() { return (SampleBuffer*)((char*)this + mSampleBufferOffset); }
                RequestDesc* AdvanceSlot(unsigned char* pSlot); //todo
                void AdvanceDecodeState(int numSamples); //todo
                RequestDesc* GetCurrentRequestDesc(); //todo
                RequestDesc* GetDecodingRequestDesc()
                {
                    RequestDesc* desc = (RequestDesc*)((char*)this + sizeof(RequestDesc) * mDecodeSlot + mRequestDescOffset);
                    if (!desc->numSamples)
                    {
                        desc = 0;
                    }
                    return desc;
                }
                void ResetPrepareSlot()
                {
                    mPrepareSlot = mDecodeSlot + 1;
                    if (mPrepareSlot >= mMaxSlots)
                    {
                        mPrepareSlot = 0;
                    }
                }
            private:
                System* mpSystemUseGetSystemAccessor;
                void* mpDecoder;
                void(*mpReleaseEvent)(Decoder*);
                void* mSampleBufferStorage;
                int(*mpDecodeEvent)(Decoder*, SampleBuffer*, int);
                unsigned int mGuid;
                int mDecodeSlotSamplesDecoded;
                unsigned int mInstanceSize;
                unsigned int mRequestDescOffset;
                unsigned int mSampleBufferOffset;
                unsigned short mDecodedSamplesAvailable;
                unsigned char mNumChannels;
                unsigned char mFeedSlot;
                unsigned char mPrepareSlot;
                unsigned char mDecodeSlot;
                unsigned char mMaxSlots;
                unsigned char mIsBlockBased;
                friend class DecoderRegistry;
            };
            struct DecoderDesc
            {
                unsigned int (*pGetSize)(unsigned int, unsigned int*);
                bool (*pCreateInstanceEvent)(Decoder*);
                void (*pReleaseEvent)(Decoder*);
                int (*pDecodeEvent)(Decoder*, SampleBuffer*, int);
                unsigned int guid;
                unsigned short maxBlockSize;
            };
            class DecoderExtended
            {
            public:
                Decoder* mpDecoder;
                SampleBuffer* mpSampleBuffer;
                void* mpSampleBufferStorage;
                void Release();
                unsigned char Feed(void*, int, Decoder::FeedType);
                int Decode(float*, int);
                int Decode(short** const, int);
                int Decode(short* pDst, int samples); //todo
                int Decode(float** pDst, int samples); //todo
                void SetDecoder(Decoder* decoder)
                {
                    mpDecoder = decoder;
                }
            private:
                static void TranslateF32ToS16(void*, void*, int);
                static DecoderExtended* CreateInstance(System*, unsigned int);
                friend class DecoderRegistry;
            };
            class Encoder;
            struct EncoderDesc
            {
                unsigned int guid;
                Encoder* (*CreateInstance)(int, int, System*);
                void* listNode;
                unsigned short maxLatency;
                unsigned char seekDataVersion;
                unsigned char isSeekable;
                unsigned char isLoopInfoRequired;
            };
            class Encoder
            {
            public:
                enum BitRateManagement {
                    BITRATEMANAGEMENT_USINGVBR = 0,
                    BITRATEMANAGEMENT_USINGCBR = 1
                };
                virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) = 0;
                virtual int Flush(unsigned char*, int*, void*, int*, int) = 0;
                virtual int GetDataRateOverhead();
                virtual int GetSeekMemoryRequired();
                virtual void Release();
                int GetEncodeMemoryRequired(int numFrames);
                float GetAverageDataRate() { return mAverageDataRate; }
                bool IsSeekable() { return mpEncoderDesc->isSeekable != 0; }
                void SetVbrQuality(float quality) {
                    mVbrQuality = quality;
                    mBitRateManagement = BITRATEMANAGEMENT_USINGVBR;
                }
                void SetCbrRate(int bitsPerSecond)
                {
                    mCbrRate = bitsPerSecond;
                    mBitRateManagement = BITRATEMANAGEMENT_USINGCBR;
                }
                void SetLoopInfo(int loopStart, int loopEnd)
                {
                    mLoopStart = loopStart;
                    mLoopEnd = loopEnd;
                }
                bool IsLoopInfoRequired() { return mpEncoderDesc->isLoopInfoRequired; }
                void SetIsChunked(bool isChunked) { mIsChunked = isChunked; }
                void SetSystem(System* system) { mpSystem = system; }
                System* GetSystem() { return mpSystem; }
                int GetChannels() { return mNumChannels; }
                void SetChannels(int channels) { mNumChannels = channels; }
                void SetSampleRate(int sampleRate) { mSampleRate = sampleRate; }
                void SetEncoderDesc(EncoderDesc* pEncoderDesc) { mpEncoderDesc = pEncoderDesc; }
                EncoderDesc* GetEncoderDesc() { return mpEncoderDesc; }
            protected:
                virtual ~Encoder();
            private:
                System* mpSystem;
                EncoderDesc* mpEncoderDesc;
                float mAverageDataRate;
                float mVbrQuality;
                int mCbrRate;
                int mLoopStart;
                int mLoopEnd;
                int mSampleRate;
                unsigned char mNumChannels;
                char mBitRateManagement;
                bool mIsChunked;
            };
            class EncoderExtended
            {
            private:
                Encoder* mpEncoder;
            public:
                int Encode(short** const, unsigned char*, int, int*, void*, int*, int);
                int Flush(unsigned char*, int*, void*, int*, int);
                float GetAverageDataRate();
                int GetEncodeMemoryRequired(int numFrames);
                int GetSeekMemoryRequired(int numFrames);
                bool IsLoopInfoRequired();
                bool IsSeekable();
                void SetCbrRate(int bitsPerSecond);
                void SetIsChunked(bool isChunked);
                void SetLoopInfo(int loopStart, int loopEnd);
                void SetVbrQuality(float quality);
                static void TranslateS16ToF32(short** pSrc, float* pDst, int channels, int numSamples);
                void Release();
                int GetDataRateOverhead() { return mpEncoder->GetDataRateOverhead(); }
                int Encode(float* pSampleData, unsigned char* pEncodedData, int numSamples, int* bytesEncoded, void* pSeekData, int* pSeekDataBytes, int numSamplesOut)
                {
                    return mpEncoder->Encode(pSampleData, pEncodedData, numSamples, bytesEncoded, pSeekData, pSeekDataBytes, numSamplesOut);
                }
                void SetEncoder(Encoder* encoder)
                {
                    mpEncoder = encoder;
                }
                Encoder* GetEncoder()
                {
                    return mpEncoder;
                }
                static EncoderExtended* CreateInstance(System* pSystem); //todo
                friend class EncoderRegistry;
            };
            class ListNode
            {
            private:
                ListNode* pnext;
            public:
                void SetNext(ListNode* pnode)
                {
                    pnext = pnode;
                }
                ListNode* GetNext()
                {
                    return pnext;
                }
            };
            class ListQueue
            {
            private:
                ListNode* phead;
                ListNode* ptail;
                int entries;
            public:
                ListQueue() : phead(nullptr), ptail(nullptr), entries(0)
                {
                }
                void Reset()
                {
                    phead = nullptr;
                    ptail = nullptr;
                    entries = 0;
                }
                ListNode* GetHead()
                {
                    return phead;
                }
                void Push(ListNode* pnode)
                {
                    pnode->SetNext(phead);
                    if (!ptail)
                    {
                        ptail = pnode;
                    }
                    phead = pnode;
                    entries++;
                }
                void PushTail(ListNode* pnode)
                {
                    pnode->SetNext(0);
                    if (phead)
                    {
                        ptail->SetNext(pnode);
                    }
                    else
                    {
                        phead = pnode;
                    }
                    ptail = pnode;
                    entries++;
                }
                ListNode* Pop()
                {
                    ListNode* node = phead;
                    if (node)
                    {
                        phead = node->GetNext();
                        if (!node->GetNext())
                        {
                            ptail = nullptr;
                        }
                        entries--;
                    }
                    return node;
                }
            };
            class DecoderRegistry
            {
            private:
                std::vector<DecoderDesc*> mDecoderDescList;
                System* mpSystem;
            public:
                static DecoderRegistry* CreateInstance(class System* pSystem);
                void* GetDecoderHandle(unsigned int guid);
                void* RegisterDecoder(DecoderDesc* pDecoderDesc);
                DecoderExtended* DecoderExtendedFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem);
                Decoder* DecoderFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem);
                void RegisterAllDecoders();
                DecoderRegistry();
                void Release();
            };
            class EncoderRegistry
            {
            private:
                std::vector<EncoderDesc*> mEncoderDescList;
                System* mpSystem;
            public:
                static EncoderRegistry* CreateInstance(class System* pSystem);
                void* GetEncoderHandle(unsigned int guid);
                void* RegisterEncoder(EncoderDesc* pEncoderDesc);
                EncoderExtended* EncoderExtendedFactory(void* encoderHandle, unsigned int numChannels, int sampleRate, System* pSystem);
                Encoder* EncoderFactory(void* encoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem); //todo
                void RegisterAllEncoders();
                EncoderRegistry();
                void Release();
            };
            enum SndPlayerCodec
            {
                SNDPLAYER_CODEC_XAS_INT = 0x0,
                SNDPLAYER_CODEC_EALAYER3_INT = 0x1,
                SNDPLAYER_CODEC_SIGN16BIG_INT = 0x2,
                SNDPLAYER_CODEC_EAXMA = 0x3,
                SNDPLAYER_CODEC_XAS1_INT = 0x4,
                SNDPLAYER_CODEC_EALAYER31_INT = 0x5,
                SNDPLAYER_CODEC_EALAYER32PCM_INT = 0x6,
                SNDPLAYER_CODEC_EALAYER32SPIKE_INT = 0x7,
                SNDPLAYER_CODEC_GCADPCM = 0x8,
                SNDPLAYER_CODEC_EASPEEX = 0x9,
                SNDPLAYER_CODEC_MAX = 0xA,
            };
            enum SndPlayerPlayType
            {
                SNDPLAYER_PLAYTYPE_RAM = 0x0,
                SNDPLAYER_PLAYTYPE_STREAM = 0x1,
                SNDPLAYER_PLAYTYPE_GIGASAMPLE = 0x2,
                SNDPLAYER_PLAYTYPE_MAX = 0x3,
            };
            struct SndPlayerHeader
            {
                int version;
                int codec;
                int channel_config;
                int sample_rate;
                int type;
                int loop_flag;
                int num_samples;
                int loop_start;
                int prefech_samples;
                int loop_offset;
            };
        }
    }
}