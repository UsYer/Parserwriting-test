#ifndef IPARSABLE_HPP_INCLUDED
#define IPARSABLE_HPP_INCLUDED

class IParsable
{
	public:
		IParsable()
		{}
		virtual ~IParsable()
		{}
		virtual void Parse(Parser::ParserContext&) = 0;
	protected:

	private:

};


#endif // IPARSABLE_HPP_INCLUDED
