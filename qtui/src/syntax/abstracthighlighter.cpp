/*
    Copyright (C) 2016 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "abstracthighlighter.h"
#include "abstracthighlighter_p.h"
#include "context_p.h"
#include "definition_p.h"
#include "format.h"
#include "qtmatcher.h"
#include "repository.h"
#include "rule_p.h"
#include "state.h"
#include "state_p.h"
#include "theme.h"

#include <cassert>
#include <iostream>

using namespace KSyntaxHighlighting;

AbstractHighlighterPrivate::AbstractHighlighterPrivate()
{
}

AbstractHighlighterPrivate::~AbstractHighlighterPrivate()
{
}

void AbstractHighlighterPrivate::ensureDefinitionLoaded()
{
    auto defData = DefinitionData::get(m_definition);
    if (!m_definition.isValid() && defData->repo && !m_definition.name().empty()) {
        std::cerr << "Definition became invalid, trying re-lookup." << std::endl;
        m_definition = defData->repo->definitionForName(m_definition.name());
        defData = DefinitionData::get(m_definition);
    }

    if (!defData->repo && !defData->name.empty())
        std::cerr << "Repository got deleted while a highlighter is still active!" << std::endl;

    if (m_definition.isValid())
        defData->load();
}

/**
 * Returns the index of the first non-space character. If the line is empty,
 * or only contains white spaces, -1 is returned.
 */
static inline int firstNonSpaceChar(const QString & text)
{
    for (int i = 0; i < text.length(); ++i) {
        if (!text[i].isSpace()) {
            return i;
        }
    }
    return -1;
}

AbstractHighlighter::AbstractHighlighter() :
    d_ptr(new AbstractHighlighterPrivate)
{
}

AbstractHighlighter::AbstractHighlighter(AbstractHighlighterPrivate *dd) :
    d_ptr(dd)
{
}

AbstractHighlighter::~AbstractHighlighter()
{
    delete d_ptr;
}

Definition AbstractHighlighter::definition() const
{
    return d_ptr->m_definition;
}

void AbstractHighlighter::setDefinition(const Definition &def)
{
    d_ptr->m_definition = def;
}

Theme AbstractHighlighter::theme() const
{
    return d_ptr->m_theme;
}

void AbstractHighlighter::setTheme(const Theme &theme)
{
    d_ptr->m_theme = theme;
}

State AbstractHighlighter::highlightLine(const QString& text, const State &state)
{
    // verify definition, deal with no highlighting being enabled
    d_ptr->ensureDefinitionLoaded();
    if (!d_ptr->m_definition.isValid()) {
        applyFormat(0, text.size(), Format());
        return State();
    }

    // verify/initialize state
    auto defData = DefinitionData::get(d_ptr->m_definition);
    auto newState = state.copy();
    auto stateData = StateData::get(newState);
    if (stateData->m_defData && defData != stateData->m_defData) {
        std::cerr << "Got invalid state, resetting." << std::endl;
        stateData->clear();
    }
    if (stateData->isEmpty()) {
        stateData->push(defData->initialContext());
        stateData->m_defData = defData;
    }

    // process empty lines
    if (text.isEmpty()) {
        while (!stateData->topContext()->lineEmptyContext().isStay())
            d_ptr->switchContext(stateData, stateData->topContext()->lineEmptyContext());
        applyFormat(0, 0, Format());
        return newState;
    }

    assert(!stateData->isEmpty());
    int firstNonSpace = firstNonSpaceChar(text);
    if (firstNonSpace < 0) {
        firstNonSpace = text.size();
    }
    int offset = 0, beginOffset = 0;
    auto currentLookupContext = stateData->topContext();
    auto currentFormat = currentLookupContext->attribute();
    bool lineContinuation = false;
    std::map<Rule*, int> skipOffsets;

    do {
        bool isLookAhead = false;
        int newOffset = 0;
        std::string newFormat;
        auto newLookupContext = currentLookupContext;
        for (const auto &rule : stateData->topContext()->rules()) {
            auto it = skipOffsets.find(rule.get());
            if (it != skipOffsets.end() && it->second > offset)
                continue;

            // filter out rules that only match for leading whitespace
            if (rule->firstNonSpace() && (offset > firstNonSpace)) {
                continue;
            }

            // filter out rules that require a specific column
            if ((rule->requiredColumn() >= 0) && (rule->requiredColumn() != offset)) {
                continue;
            }

            QtMatcher m(text);
            const auto newResult = rule->match(m, offset);
            newOffset = newResult.offset();
            if (newResult.skipOffset() > newOffset)
                skipOffsets.insert(std::make_pair(rule.get(), newResult.skipOffset()));
            if (newOffset <= offset)
                continue;

            if (rule->isLookAhead()) {
                assert(!rule->context().isStay());
                d_ptr->switchContext(stateData, rule->context());
                isLookAhead = true;
                break;
            }

            newLookupContext = stateData->topContext();
            d_ptr->switchContext(stateData, rule->context());
            newFormat = rule->attribute().empty() ? stateData->topContext()->attribute() : rule->attribute();
            if (newOffset == text.size() && std::dynamic_pointer_cast<LineContinue>(rule))
                lineContinuation = true;
            break;
        }
        if (isLookAhead)
            continue;

        if (newOffset <= offset) { // no matching rule
            if (stateData->topContext()->fallthrough()) {
                d_ptr->switchContext(stateData, stateData->topContext()->fallthroughContext());
                continue;
            }

            newOffset = offset + 1;
            newLookupContext = stateData->topContext();
            newFormat = newLookupContext->attribute();
        }

        if (newFormat != currentFormat /*|| currentLookupDef != newLookupDef*/) {
            if (offset > 0)
                applyFormat(beginOffset, offset - beginOffset, currentLookupContext->formatByName(currentFormat));
            beginOffset = offset;
            currentFormat = newFormat;
            currentLookupContext = newLookupContext;
        }
        assert(newOffset > offset);
        offset = newOffset;

    } while (offset < text.size());

    if (beginOffset < offset)
        applyFormat(beginOffset, text.size() - beginOffset, currentLookupContext->formatByName(currentFormat));

    while (!stateData->topContext()->lineEndContext().isStay() && !lineContinuation) {
        if (!d_ptr->switchContext(stateData, stateData->topContext()->lineEndContext()))
            break;
    }

    return newState;
}

bool AbstractHighlighterPrivate::switchContext(StateData *data, const ContextSwitch &contextSwitch)
{
    for (int i = 0; i < contextSwitch.popCount(); ++i) {
        // don't pop the last context if we can't push one
        if (data->size() == 1 && !contextSwitch.context())
            return false;
        if (data->size() == 0)
            break;
        data->pop();
    }

    if (contextSwitch.context())
        data->push(contextSwitch.context());

    assert(!data->isEmpty());
    return true;
}

