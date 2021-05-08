#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <new>
#include <algorithm>
#include "eabits.h"

namespace EA {
    namespace Allocator {
        ICoreAllocator::~ICoreAllocator()
        {
        }
        void* ICoreAllocator::Alloc(unsigned int size, const char* name, unsigned int flags, unsigned int align, unsigned int alignOffset)
        {
            return _aligned_malloc(size, align);
        }
        void* ICoreAllocator::Alloc(unsigned int size, const char* name, unsigned int flags)
        {
            return _aligned_malloc(size, 4);
        }
        void ICoreAllocator::Free(void* block, unsigned int size)
        {
            return _aligned_free(block);
        }
        void* ICoreAllocator::AllocDebug(unsigned int size, DebugParams debugParams, unsigned int flags, unsigned int align, unsigned int alignOffset)
        {
            return Alloc(size, debugParams.mName, flags, align, alignOffset);
        }
        void* ICoreAllocator::AllocDebug(unsigned int size, DebugParams debugParams, unsigned int flags)
        {
            return Alloc(size, debugParams.mName, flags);
        }
    }
}

namespace rw {
    namespace audio {
        namespace core {
            class EALayer3Core;
            class EaLayer3DecBase : public Decoder {
            protected:
                static void ReleaseEvent(Decoder*);
                static unsigned int GetSize(unsigned int, unsigned int*);
                static bool CreateInstance(class rw::audio::core::Decoder*, char);
            private:
                void DecodePcm(float**, void*, unsigned int, unsigned int);
                int DecodeSpecialBlock(unsigned char*, float**, EALayer3Core*);
                void SkipBlocks();
                int DecodeGranule(unsigned char*, float**, EALayer3Core*, int*, int*, int*, int);
            public:
                static int DecodeEvent(Decoder*, SampleBuffer*, int);
            private:
                EALayer3Core* mpLoadedEALayer3Core;
                unsigned char* mpEncodedSample;
                unsigned char* mpEncodedDataEnd;
                rw::audio::core::EALayer3Core** mppEaLayer3Core;
                int mRemainingSamples;
                int mTotalChannels;
                int mNumEaLayer3CoreInstances;
                int mNextBlockSize;
                int mLatency;
                int mSkipSamples;
                char mVersion;
                bool mNewFeed;
            };
            class EaLayer31Dec : public EaLayer3DecBase {
            private:
                static bool CreateInstanceEvent(Decoder*);
                static DecoderDesc sDecoderDesc;
            public:
                static DecoderDesc* GetDecoderDesc();
            };
            DecoderDesc EaLayer31Dec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'EL31', 576 };
            class EaLayer32PcmDec : public EaLayer3DecBase {
            private:
                static bool CreateInstanceEvent(Decoder*);
                static DecoderDesc sDecoderDesc;
            public:
                static DecoderDesc* GetDecoderDesc();
            };
            DecoderDesc EaLayer32PcmDec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'L32P', 576 };
            class EaLayer32SpikeDec : public EaLayer3DecBase {
            private:
                static bool CreateInstanceEvent(Decoder*);
                static DecoderDesc sDecoderDesc;
            public:
                static DecoderDesc* GetDecoderDesc();
            };
            DecoderDesc EaLayer32SpikeDec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'L32S', 576 };
            class Pcm16BigDec : public Decoder {
            private:
                static unsigned int GetSize(unsigned int, unsigned int*);
                static bool CreateInstanceEvent(Decoder*);
                static DecoderDesc sDecoderDesc;
            public:
                static int DecodeEvent(Decoder*, SampleBuffer*, int);
                static DecoderDesc* GetDecoderDesc();
            private:
                unsigned int mpSrc;
                int mRemainingSamples;
            };
            DecoderDesc Pcm16BigDec::sDecoderDesc = { GetSize, CreateInstanceEvent, nullptr, DecodeEvent, 'P6B0', 0 };
            class Xas1Dec {
            private:
                static const float sShiftMulLut[13];
                static const float sFilterPairs[4][2];
                static unsigned int GetSize(unsigned int, unsigned int*);
                static bool CreateInstanceEvent(Decoder*);
                static DecoderDesc sDecoderDesc;
                void DecodeChannel(unsigned char*, float*);
            public:
                static int DecodeEvent(Decoder*, SampleBuffer*, int);
                static DecoderDesc* GetDecoderDesc();
            };
            DecoderDesc Xas1Dec::sDecoderDesc = { GetSize, CreateInstanceEvent, nullptr, DecodeEvent, 'Xas1', 128 };
            class Layer3Dec {
            private:
                static const float sShiftMulLut[13];
                static const float sFilterPairs[4][2];
                static unsigned int GetSize(unsigned int, unsigned int*);
                static void ReleaseEvent(Decoder*);
                static bool CreateInstanceEvent(Decoder*);
                static DecoderDesc sDecoderDesc;
                void DecodeChannel(unsigned char*, float*);
            public:
                static int DecodeEvent(Decoder*, SampleBuffer*, int);
                static DecoderDesc* GetDecoderDesc();
            };
            DecoderDesc Layer3Dec::sDecoderDesc = { GetSize, CreateInstanceEvent, ReleaseEvent, DecodeEvent, 'MP30', 1152 };
            struct Xas1Enc : public Encoder
            {
            public:
                static EncoderDesc* GetEncoderDesc();
                virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) override;
                virtual int Flush(unsigned char*, int*, void*, int*, int) override;
            private:
                static Encoder* CreateInstance(int, int, System*);
                void CustomInterleaveXAS(unsigned char*, unsigned char*);
                void EncodeBlock(float*, unsigned char*, int);
                static EncoderDesc sEncoderDesc;
                static const float sFilterPairs[4][2];
                int mResidualSamples;
                float mResidueBuffer[8192];
            };
            EncoderDesc Xas1Enc::sEncoderDesc = { 'Xas1', CreateInstance, nullptr, 0, 0, 1, 0 };
            struct Pcm16BigEnc : public Encoder
            {
            public:
                static EncoderDesc* GetEncoderDesc();
                virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) override;
                virtual int Flush(unsigned char*, int*, void*, int*, int) override;
            private:
                static Encoder* CreateInstance(int, int, System*);
                static EncoderDesc sEncoderDesc;
            };
            EncoderDesc Pcm16BigEnc::sEncoderDesc = { 'P6B0', CreateInstance, nullptr, 0, 0, 1, 0 };
            struct EaLayer32Enc : public Encoder
            {
            public:
                virtual int Encode(float*, unsigned char*, int, int*, void*, int*, int) override;
                virtual int Flush(unsigned char*, int*, void*, int*, int) override;
                virtual int GetSeekMemoryRequired() override;
                virtual void Release() override;
            };
            struct EaLayer32PcmEnc : public EaLayer32Enc
            {
            public:
                static EncoderDesc* GetEncoderDesc();
            private:
                static Encoder* CreateInstance(int, int, System*);
                static EncoderDesc sEncoderDesc;
            };
            EncoderDesc EaLayer32PcmEnc::sEncoderDesc = { 'L32P', CreateInstance, nullptr, 1728, 0, 1, 0 };
            struct EaLayer32SpikeEnc : public Encoder
            {
            public:
                static EncoderDesc* GetEncoderDesc();
            private:
                static Encoder* CreateInstance(int, int, System*);
                static EncoderDesc sEncoderDesc;
            };
            EncoderDesc EaLayer32SpikeEnc::sEncoderDesc = { 'L32S', CreateInstance, nullptr, 1728, 0, 1, 0 };
            struct EaLayer31Enc : public Encoder
            {
            public:
                static EncoderDesc* GetEncoderDesc();
            private:
                static Encoder* CreateInstance(int, int, System*);
                static EncoderDesc sEncoderDesc;
            };
            EncoderDesc EaLayer31Enc::sEncoderDesc = { 'EL31', CreateInstance, nullptr, 1728, 0, 1, 0 };
            System::System()
            {
                mpStackAllocator = nullptr;
                mpAssertImplementation = Assert;
            }
            void System::CreateInstance(EA::Allocator::ICoreAllocator* pAllocator)
            {
                void* v = pAllocator->Alloc(sizeof(System), "rw::audio::core::System", 1, 16, 0);
                System* s = new(v) System;
                s->mpCommandBuffer = nullptr;
                spInstance = s;
                s->mpAllocator = pAllocator;
                s->mpPhysicalAlloc = nullptr;
                s->mpPhysicalFree = nullptr;
                s->mSystemTime = 0.0;
                s->mpMasteringSubMix = nullptr;
                s->mpPlugInRegistry = nullptr;
                s->mpDecoderRegistry = nullptr;
                s->mpEncoderRegistry = nullptr;
            }
            void System::Assert(System*, const char*, const char*, const char*, int)
            {
            }
            void* System::Alloc(unsigned int size, char const* name, unsigned int alignment, EA::Allocator::ICoreAllocator* pAllocator)
            {
                EA::Allocator::ICoreAllocator* a = pAllocator;
                if (!pAllocator)
                {
                    a = mpAllocator;
                }
                return a->Alloc(size, name, 1, alignment, 0);
            }
            void System::Free(void* pMem, EA::Allocator::ICoreAllocator* pAllocator)
            {
                EA::Allocator::ICoreAllocator* a = pAllocator;
                if (!pAllocator)
                {
                    a = mpAllocator;
                }
                a->Free(pMem, 0);
            }
            void System::Lock()
            {
            }
            void System::Unlock()
            {
            }
            System* System::spInstance = nullptr;
            DecoderRegistry* System::GetDecoderRegistry()
            {
                if (!mpDecoderRegistry)
                {
                    mpDecoderRegistry = DecoderRegistry::CreateInstance(spInstance);
                }
                return mpDecoderRegistry;
            }
            EncoderRegistry* System::GetEncoderRegistry()
            {
                if (!mpEncoderRegistry)
                {
                    mpEncoderRegistry = EncoderRegistry::CreateInstance(spInstance);
                }
                return mpEncoderRegistry;
            }
            void System::Release()
            {
                if (mpDecoderRegistry)
                {
                    mpDecoderRegistry->Release();
                }
                if (mpEncoderRegistry)
                {
                    mpEncoderRegistry->Release();
                }
                mpAllocator->Free(this, 0);
                spInstance = nullptr;
            }
            DecoderRegistry* DecoderRegistry::CreateInstance(class System* pSystem)
            {
                void* v = pSystem->Alloc(16, "rw::audio::core::DecoderRegistry", 16, 0);
                DecoderRegistry* d = new(v) DecoderRegistry();
                d->mpSystem = pSystem;
                return d;
            }
            void* DecoderRegistry::GetDecoderHandle(unsigned int guid)
            {
                for (DecoderDesc* d : mDecoderDescList)
                {
                    if (d->guid == guid)
                    {
                        return d;
                    }
                }
                return nullptr;
            }
            void* DecoderRegistry::RegisterDecoder(DecoderDesc* pDecoderDesc)
            {
                if (std::find(mDecoderDescList.begin(), mDecoderDescList.end(), pDecoderDesc) == mDecoderDescList.end())
                {
                    mDecoderDescList.push_back(pDecoderDesc);
                }
                return pDecoderDesc;
            }
            DecoderExtended* DecoderRegistry::DecoderExtendedFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem)
            {
                DecoderExtended* decoder = DecoderExtended::CreateInstance(pSystem, numChannels);
                decoder->SetDecoder(DecoderFactory(decoderHandle, numChannels, maxRequests, pSystem));
                return decoder;
            }
            Decoder* DecoderRegistry::DecoderFactory(void* decoderHandle, unsigned int numChannels, unsigned int maxRequests, System* pSystem)
            {
                DecoderDesc* desc = (DecoderDesc*)decoderHandle;
                unsigned int alignment;
                unsigned int size = desc->pGetSize(numChannels, &alignment);
                unsigned int size2 = sizeof(Decoder::RequestDesc) * maxRequests + ((size + 7) & 0xFFFFFFF8);
                bool isblock = desc->maxBlockSize != 0;
                if (isblock)
                {
                    size2 = ((size2 + 15) & 0xFFFFFFF0) + 20;
                    if (alignment <= 16)
                    {
                        alignment = 16;
                    }
                }
                if (!size2)
                {
                    size2 = 52;
                }
                void* v = System::GetInstance()->Alloc(size2, "rw::audio::core::Decoder", alignment, nullptr);
                Decoder* d = new(v) Decoder;
                d->mpReleaseEvent = desc->pReleaseEvent;
                d->mSampleBufferStorage = nullptr;
                d->mNumChannels = (unsigned char)numChannels;
                d->mpSystemUseGetSystemAccessor = pSystem;
                desc->pCreateInstanceEvent(d);
                d->mpDecoder = d;
                d->mpDecodeEvent = desc->pDecodeEvent;
                d->mMaxSlots = (unsigned char)maxRequests;
                d->mInstanceSize = size2;
                d->mGuid = desc->guid;
                d->mDecodedSamplesAvailable = 0;
                d->mDecodeSlotSamplesDecoded = 0;
                d->mFeedSlot = 0;
                d->mPrepareSlot = 0;
                d->mDecodeSlot = 0;
                d->mIsBlockBased = isblock;
                unsigned int offset = ((unsigned int)&d->mpSystemUseGetSystemAccessor + size + 3) & 0xFFFFFFF8;
                d->mRequestDescOffset = offset - (unsigned int)d;
                if (isblock)
                {
                    SampleBuffer* buffer = (SampleBuffer*)((offset + sizeof(Decoder::RequestDesc) * maxRequests + 15) & 0xFFFFFFF0);
                    d->mSampleBufferOffset = (unsigned int)((unsigned int)buffer - (unsigned int)d);
                    d->mSampleBufferStorage = pSystem->Alloc(4 * numChannels * desc->maxBlockSize, "Decoder block storage", 128, nullptr);
                    SampleBuffer::CreateInstance(numChannels, 0, desc->maxBlockSize, buffer, d->mSampleBufferStorage, pSystem);
                }
                Decoder::RequestDesc* request = (Decoder::RequestDesc*)((char*)d + d->mRequestDescOffset);
                for (unsigned int i = 0; i < d->mMaxSlots; i++)
                {
                    request->pSrc = 0;
                    request->numSamples = 0;
                    request++;
                }
                return d;
            }
            void DecoderRegistry::RegisterAllDecoders()
            {
                RegisterDecoder(Xas1Dec::GetDecoderDesc());
                RegisterDecoder(Pcm16BigDec::GetDecoderDesc());
                RegisterDecoder(Layer3Dec::GetDecoderDesc());
                RegisterDecoder(EaLayer31Dec::GetDecoderDesc());
                RegisterDecoder(EaLayer32PcmDec::GetDecoderDesc());
                RegisterDecoder(EaLayer32SpikeDec::GetDecoderDesc());
            }
            DecoderRegistry::DecoderRegistry()
            {
            }
            void DecoderRegistry::Release()
            {
                mpSystem->Free(this, nullptr);
            }
            EncoderRegistry* EncoderRegistry::CreateInstance(class System* pSystem)
            {
                void* v = pSystem->Alloc(16, "rw::audio::core::EncoderRegistry", 16, 0);
                EncoderRegistry* e = new(v) EncoderRegistry();
                e->mpSystem = pSystem;
                return e;
            }
            void* EncoderRegistry::GetEncoderHandle(unsigned int guid)
            {
                for (EncoderDesc* e : mEncoderDescList)
                {
                    if (e->guid == guid)
                    {
                        return e;
                    }
                }
                return nullptr;
            }
            void* EncoderRegistry::RegisterEncoder(EncoderDesc* pEncoderDesc)
            {
                if (std::find(mEncoderDescList.begin(), mEncoderDescList.end(), pEncoderDesc) == mEncoderDescList.end())
                {
                    mEncoderDescList.push_back(pEncoderDesc);
                }
                return pEncoderDesc;
            }
            EncoderExtended* EncoderRegistry::EncoderExtendedFactory(void* encoderHandle, unsigned int numChannels, int sampleRate, System* pSystem)
            {
                EncoderExtended* encoder = (EncoderExtended *)pSystem->Alloc(4, "rw::audio::core::EncoderExtended", 0x10, nullptr);
                if (encoder)
                {
                    EncoderDesc* desc = (EncoderDesc*)encoderHandle;
                    Encoder* enc = desc->CreateInstance(numChannels, sampleRate, pSystem);
                    encoder->SetEncoder(enc);
                    encoder->mpEncoder->SetSystem(pSystem);
                    encoder->mpEncoder->SetChannels(numChannels);
                    encoder->mpEncoder->SetSampleRate(sampleRate);
                    encoder->mpEncoder->SetEncoderDesc(desc);
                }
                return encoder;
            }
            void EncoderRegistry::RegisterAllEncoders()
            {
                RegisterEncoder(Xas1Enc::GetEncoderDesc());
                RegisterEncoder(Pcm16BigEnc::GetEncoderDesc());
                RegisterEncoder(EaLayer31Enc::GetEncoderDesc());
                RegisterEncoder(EaLayer32PcmEnc::GetEncoderDesc());
                RegisterEncoder(EaLayer32SpikeEnc::GetEncoderDesc());
            }
            EncoderRegistry::EncoderRegistry()
            {
            }
            void EncoderRegistry::Release()
            {
                mpSystem->Free(this, nullptr);
            }
            class BitGetter
            {
            public:
                BitGetter() {}
                ~BitGetter() {}
                void SetBitBuffer(void* pBuffer)
                {
                    mpBitBuffer = (unsigned char*)pBuffer;
                    mBitPosition = 0;
                }
                unsigned int GetBitPosition() { return mBitPosition; }
                unsigned int GetBits(unsigned int numBits)
                {
                    unsigned int i1 = 0;
                    if (!numBits)
                    {
                        return 0;
                    }
                    do
                    {
                        unsigned int i2 = mBitPosition & 7;
                        unsigned int i3 = 8 - i2;
                        if (8 - i2 > numBits)
                        {
                            i3 = numBits;
                        }
                        unsigned int i4 = mpBitBuffer[mBitPosition >> 3] >> (8 - i2 - i3);
                        mBitPosition += i3;
                        i1 = (i1 << i3) | ((1 << i3) - 1) & i4;
                        numBits -= i3;
                    } while (numBits);
                    return i1;
                }
            private:
                unsigned char* mpBitBuffer;
                unsigned int mBitPosition;
            };
        }
    }
}

bool mIsSse = true;

extern "C" void memset2(void* v, int i, size_t s)
{
    memset(v, i, s);
}

extern "C" void memcpy2(void* v, const void* v2, size_t s)
{
    memcpy(v, v2, s);
}
