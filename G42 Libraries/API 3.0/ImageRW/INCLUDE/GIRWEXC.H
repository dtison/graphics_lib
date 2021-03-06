// girwexc.h - image read write exceptions
#if !defined(GIRWEXC_H)
#define GIRWEXC_H
class G42IRWInvalidImageException : public G42Exception
{
public:
	G42IRWInvalidImageException(uintf error_id = 0)
			: G42Exception(error_id) {}
};
class G42IRWNotAnImageException : public G42Exception
{
public:
	G42IRWNotAnImageException(uintf error_id = 0)
			: G42Exception(error_id) {}
};
#endif // GIRWEXC_H
