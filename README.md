# Stencil Lang

Linguagem para criação de Pixel Arts com loops.

> Inspirada no jogo [Replicube](https://store.steampowered.com/app/3401490/Replicube/)

## Como utilizar

### Stencils

```py
# Todo o código dentro de um stencil é executado uma vez
# para cada pixel onde ele está sendo aplicado.

# Stencils usam o comando `paint` para pintar o pixel atual
# de uma determinada cor. Utilizar o `paint` termina a execução
# do loop e passa para o próximo pixel.

# Os valores de x e y são preenchidos automaticamente pelo
# interpretador com as coordenadas do pixel atual.

stencil main {
    paint x;
}
```

### Aplicando Stencils
```py
# Stencils podem ser aplicados através
# do comando `apply`
apply main;

# Também é possível aplicar stencils em uma
# determinada região do canvas.
apply main at [0, 0];

# E especificar o tamanho da área em que ele será aplicado.
apply main at [0, 0] size 100;

# Stencils sem tamanho e sem posição são aplicados
# no centro do canvas e em toda a sua área.
```

### Funções

```py
# Funções podem ser definidas através do comando `func`. Elas
# podem executar qualquer código, inclusive outras funções, e
# devem retornar um valor inteiro.
func add(a, b) {
    return a + b;
}

# É possível utilizar funções dentro de stencils, ou no apply:
apply main at [add(1, 2), add(3, 4)]; # equivalente a apply main at [3, 7]
```

### Variáveis

```py
# Variáveis podem ser definidas através do comando `var`,
# e só podem armazenar valores inteiros.
var a = 1;
var b = 2;

# É possível utilizar variáveis em stencils, ou no apply:
apply main at [a, b];
```

## Gramática

```ebnf
<Expression> ::= <Term> (("+" | "-" | "||") <Term>)*
<RelExp> ::= <Expression> (("==" | ">" | "<") <Expression>)*
<Term> ::= <Factor> (("*" | "/" | "&&") <Factor>)*
<Number> ::= <Digit>+
<Factor> ::= <Number> | (("+" | "-" | "!") <Factor>) | ("(" <RelExp> ")") | <Identifier> | <FuncCall>
<Digit> ::= [0-9]
<Letter> ::= [a-z] | [A-Z]
<Assignment> ::= <Identifier> "=" <RelExp>
<VarDec> ::= "var" <Identifier> "=" <Expression>
<Block> ::= <RBracket> <Statement>+ <LBracket>
<RBracket> ::= "{"
<LBracket> ::= "}"
<Statement> ::= ((<Assignment> | <FuncCall>) ";") | <Block> | <If> | <FuncDec> | <Return> | <VarDec> | <Paint> | <Apply> | <Stencil>
<If> ::= "if" "(" <RelExp> ")" <Statement> ("else" <Statement>)?
<FuncDec> ::= "func" <Identifier> "(" <VarDec>? ")" <Block>
<FuncCall> ::= <Identifier> "(" (<Expression> ("," <Expression>)*)? ")"
<Identifier> ::= <Letter>+
<Stencil> ::= "stencil" <Identifier> <Block>
<Apply> ::= "apply" <Identifier> <Directive>*
<Coordinate> ::= "[" <Factor> "," <Factor> "]"
<Directive> ::= <LocationDirective> | <SizeDirective>
<LocationDirective> ::= "at" <Coordinate>
<SizeDirective> ::= "size" <Factor>
<Paint> ::= "paint" <Factor>
<Return> ::= "return" <Expression>
```