#ifndef RUNTIMEFUNC_HPP_INCLUDED
#define RUNTIMEFUNC_HPP_INCLUDED
namespace Internal
{
class RuntimeFunc: public IFunction
{
    class FuncEvaluator : public boost::static_visitor<>
    {
        Types::Stack m_Stack;
        EvaluationContext m_EC;

        public:
        FuncEvaluator(MemoryController& MC, Types::Scope& LocalScope):
            m_EC(m_Stack, LocalScope, MC)
        {}
        void operator()(long long Val)
        {
            m_Stack.push_back(Types::Object(Val));
        }
        void operator()(double val)
        {
            m_Stack.push_back(Types::Object(val));
        }
        void operator()(const boost::shared_ptr<IEvaluable>& Evaluable)
        {
            Evaluable->Eval(m_EC);
        }
        void operator()(const std::string& s)
        {
            m_Stack.push_back(Types::Object(s));
        }
    };
    public:
    std::deque<Token> m_FuncInstructions;
    RuntimeFunc(const std::string& Name, const std::string& Representation, const std::vector<std::string>& Args, unsigned ReturnCount):
        IFunction(Name, Representation, Args.size(), ReturnCount)
    {
        foreach( const std::string& Arg, Args )
        {
            m_LocalScope[Arg];
        }
    }
    virtual void Eval(EvaluationContext& EC)
    {
        CountedReference LocalScopeRef(EC.MC.Save(m_LocalScope));
        (*LocalScopeRef)["__PARENT__"] = EC.Scope;
        (*LocalScopeRef)["This"] = m_This;
        FuncEvaluator FE(EC.MC,LocalScopeRef);
        foreach(Token& T,m_FuncInstructions)
        {
        #ifdef DEBUG
            std::cout << boost::apply_visitor(Utilities::PrintValueNoResolve(),T) << " ";
        #endif
            boost::apply_visitor(FE,T);
        }
        EC.EvalStack.push_back(Types::Object((*LocalScopeRef)["__RETURN__"]));
    }
};
}//ns Internal


#endif // RUNTIMEFUNC_HPP_INCLUDED
