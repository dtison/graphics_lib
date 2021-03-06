// girwopt.h - image read/write options
#if !defined(GIRWOPT_H)
#define GIRWOPT_H
#include "gnotify.h"
#include "gpobj.h"
#include "gopbase.h"
class G42ImageReadWriteOptions : public G42BaseOptions
{
public:
	G42ImageReadWriteOptions(intf type_id, bool compressed = false,
		byte true_color_depth = 24, bool progressive = false,
		bool write = true)
	:
		G42BaseOptions(type_id, write), Compressed(compressed),
		Progressive(progressive), TrueColorDepth(true_color_depth)
			{}
	virtual ~G42ImageReadWriteOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42ImageReadWriteOptions(*this);}
	bool IsCompressed(void) const
		{ return Compressed; }
	bool IsProgressive(void) const
		{ return Progressive; }
	byte GetTrueColorDepth(void) const
		{ return TrueColorDepth; }
	void SetCompressed(bool compressed)
		{ Compressed = compressed; }
	void SetProgressive(bool progressive)
		{ Progressive = progressive; }
	void SetTrueColorDepth(byte depth)
		{ TrueColorDepth = depth; }
protected:
	bool Compressed;
	byte TrueColorDepth; // 16, 24, 32
	bool Progressive;
	G42ImageReadWriteOptions(G42ImageReadWriteOptions & ref)
	:
		G42BaseOptions(ref), Compressed(ref.Compressed),
		Progressive(ref.Progressive), TrueColorDepth(ref.TrueColorDepth)
			{}
private:
	// Hidden operator=
	G42ImageReadWriteOptions & operator=(G42ImageReadWriteOptions &)
		{return *this;}
};
class G42ImageReadOptions : public G42BaseOptions
{
public:
	G42ImageReadOptions(bool displaying_progressively = false,
		bool reading_animated = true)
	:
		G42BaseOptions(0, false),
			DisplayingProgressively(displaying_progressively),
			ReadingAnimated(reading_animated)
			{}
	virtual ~G42ImageReadOptions(void) {}
	virtual G42BaseOptions * MakeCopy(void)
		{return new G42ImageReadOptions(*this);}
	bool IsDisplayingProgressively(void) const
		{ return DisplayingProgressively; }
	bool IsReadingAnimated(void) const
		{ return ReadingAnimated; }
protected:
	bool DisplayingProgressively;
	bool ReadingAnimated;
	G42ImageReadOptions(G42ImageReadOptions & ref)
	:
		G42BaseOptions(ref),
		DisplayingProgressively(ref.DisplayingProgressively),
		ReadingAnimated(ref.ReadingAnimated)
			{}
private:
	// Hidden operator=
	G42ImageReadOptions & operator = (G42ImageReadOptions &)
		{return *this;}
};
#endif // GIRWOPT_H sentry
