
#ifndef G42ARRAY_H
#define G42ARRAY_H
// note: Block must be a power of 2
template <class T>
class G42Array
{
	protected:
		uint NumIndex;
		T * Array;
		uint Block;
	public:
		T & operator [] (int index) {
			if (index < 0)
				index = 0;
			if (index >= NumIndex)
				ExpandArray(index + 1);
			if (index >= NumIndex)
				index = NumIndex - 1;
			return Array[index]; }
		T & operator [] (int index) const {
			if (index >= NumIndex)
				index = NumIndex - 1;
			if (index < 0)
				index = 0;
			return Array[index];
		}
		operator T * () const
			{return Array;}
		void ExpandArray(uint number);
		void Clear(void)
		{
			NumIndex = 0;
			if (Array)
				delete[] Array;
			Array = 0;
		}
		G42Array(uint block = 16);
		~G42Array()
			{Clear();}
};
template <class T>
G42Array<T>::G42Array(uint block) :
	Array(0), NumIndex(0), Block(1)
{
	while (Block < block) // normalize Block to next larger power of two
	{
		Block <<= 1;
	}
}
template <class T>
void
G42Array<T>::ExpandArray(uint number)
{
	if (number > NumIndex)
	{
		int NewNum = (number + (Block - 1)) & (~(Block - 1));
		if (NewNum < number)
			NewNum = number;
		T * temp = Array;
		Array = new T [NewNum];
		if (Array && temp)
		{
			int i = 0;
			for (; i < NumIndex; i++)
				Array[i] = temp[i];
			delete[] temp;
			temp = 0;
		}
		if (Array)
			NumIndex = NewNum;
		else
			Array = temp;
	}
}
#endif
