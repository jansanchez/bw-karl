#pragma once

class AbstractEvent;
class ActionEvent;
class FrameEvent;
class BroodwarEvent;
class UnitUpdateEvent;
class UnitCreateEvent;

class AbstractEventVisitor
{
    public:
        virtual void visitActionEvent(ActionEvent* event) = 0;
        virtual void visitFrameEvent(FrameEvent* event) = 0;
        virtual void visitBroodwarEvent(BroodwarEvent* event) = 0;
        virtual void visitUnitUpdateEvent(UnitUpdateEvent* event) = 0;
        virtual void visitUnitCreateEvent(UnitCreateEvent* event) = 0;
};

class BasicEventVisitor : public AbstractEventVisitor
{
    public:
        void visitActionEvent(ActionEvent* event) override;
        void visitFrameEvent(FrameEvent* event) override;
        void visitBroodwarEvent(BroodwarEvent* event) override;
        void visitUnitUpdateEvent(UnitUpdateEvent* event) override;
        void visitUnitCreateEvent(UnitCreateEvent* event) override;
};
