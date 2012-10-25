#ifndef __MAGICPROPS_H__
#define __MAGICPROPS_H__

enum MAGIC_TYPE
{
	ICE			= 0x0,
	KILL_ALL	= 0x1
};

class MagicProps
{
public:
	MagicProps(void);
	~MagicProps(void);
};

#endif // end __MAGICPROPS_H__